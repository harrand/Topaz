#if TZ_VULKAN
#include "tz/gl/impl/vulkan/renderer2.hpp"
#include "tz/core/profile.hpp"
#include "tz/gl/device.hpp"
#include "tz/gl/output.hpp"

namespace tz::gl
{
	unsigned int renderer_vulkan_base::uid_counter = 0;

	struct edit_side_effects
	{
		bool rewrite_buffer_descriptors = false;
		bool rewrite_image_descriptors = false;
		bool recreate_pipeline = false;
		bool rerecord_work_commands = false;
		bool rewrite_static_resources = false;
	};

//--------------------------------------------------------------------------------------------------
	renderer_resource_manager::renderer_resource_manager(const tz::gl::renderer_info& rinfo):
	AssetStorageCommon<iresource>(rinfo.get_resources())
	{
		this->patch_resource_references(rinfo);
		this->setup_dynamic_resource_spans();
		this->populate_image_resource_views();
		this->populate_image_resource_samplers();
	}

	unsigned int renderer_resource_manager::resource_count() const
	{
		return AssetStorageCommon<iresource>::count();
	}

	const iresource* renderer_resource_manager::get_resource(tz::gl::resource_handle rh) const
	{
		return AssetStorageCommon<iresource>::get(rh);
	}

	iresource* renderer_resource_manager::get_resource(tz::gl::resource_handle rh)
	{
		return AssetStorageCommon<iresource>::get(rh);
	}

	const icomponent* renderer_resource_manager::get_component(tz::gl::resource_handle rh) const
	{
		if(rh == tz::nullhand)
		{
			return nullptr;
		}
		return this->components[static_cast<std::size_t>(static_cast<tz::hanval>(rh))].get();
	}

	icomponent* renderer_resource_manager::get_component(tz::gl::resource_handle rh)
	{
		if(rh == tz::nullhand)
		{
			return nullptr;
		}
		return this->components[static_cast<std::size_t>(static_cast<tz::hanval>(rh))].get();
	}

	std::span<vk2::ImageView> renderer_resource_manager::get_image_resource_views()
	{
		return this->image_resource_views;
	}

	std::span<vk2::Sampler> renderer_resource_manager::get_image_resource_samplers()
	{
		return this->image_resource_samplers;
	}

	void renderer_resource_manager::notify_image_dirty(tz::gl::resource_handle rh)
	{
		TZ_PROFZONE("renderer_resource_manager - notify image dirty", 0xFFAAAA00);
		// we need to know which image view index this corresponds to. just count resource image count until we reach rh vallue.
		std::size_t imgview_idx = 0;
		for(std::size_t i = 0; i < static_cast<std::size_t>(static_cast<tz::hanval>(rh)); i++)
		{
			if(this->get_resource(static_cast<tz::hanval>(i))->get_type() != tz::gl::resource_type::image)
			{
				imgview_idx++;
			}
		}
		// just replace the image view.
		tz::assert(imgview_idx < this->get_image_resource_views().size());
		vk2::Image& img = static_cast<image_component_vulkan*>(this->get_component(rh))->vk_get_image();
		this->get_image_resource_views()[imgview_idx] =
		{{
			.image = &img,
			.aspect = vk2::derive_aspect_from_format(img.get_format()).front()
		}};
	}

	void renderer_resource_manager::reseat_resource(tz::gl::renderer_edit::resource_reference resref)
	{
		// an existing component exists at resref.resource. we want to entirely replace it with the component specified.
		iresource* newres = resref.component->get_resource();
		tz::assert(newres != nullptr);
		AssetStorageCommon<iresource>::set(resref.resource, newres);
		auto resid = static_cast<std::size_t>(static_cast<tz::hanval>(resref.resource));
		tz::assert(resid < this->components.size());
		this->components[resid] = resref.component;
		// note: if descriptors have already been written, the descriptor associated with this resource is now out-of-date and needs to be rewritten.
	}

	void renderer_resource_manager::patch_resource_references(const tz::gl::renderer_info& rinfo)
	{
		TZ_PROFZONE("render_resource_manager - patch resource references", 0xFFAAAA00);
		// AssetStorageCommon populates our set of resources already from the renderer info.
		// However, if the renderer info contained resource references (rinfo.ref_resource), the resource will be nullptr, and we will need to patch it up ourselves now.
		this->components.reserve(this->resource_count());
		std::size_t resource_reference_counter = 0;
		for(std::size_t i = 0; i < this->resource_count(); i++)
		{
			tz::gl::resource_handle rh{static_cast<tz::hanval>(i)};
			iresource* res = this->get_resource(rh);
			// if resource is nullptr, then we get the next component in the list and patch that in.
			if(res == nullptr)
			{
				// find the component which should be in this resource slot.
				icomponent* comp = const_cast<icomponent*>(rinfo.get_components()[resource_reference_counter++]);
				// add the resource reference.
				this->components.push_back(comp);
				AssetStorageCommon<iresource>::set(rh, comp->get_resource());
			}
			else // otherwise, we create the component ourselves.
			{
				// make the component based on the resource.
				this->components.push_back(this->make_component_from(res));
			}
		}
	}

	void renderer_resource_manager::setup_dynamic_resource_spans()
	{
		TZ_PROFZONE("render_resource_manager - setup dynamic spans", 0xFFAAAA00);
		// go through each component. if its a dynamic resource, link the underlying image/buffer mapped data with the resource data.
		for(std::size_t i = 0; i < this->resource_count(); i++)
		{
			icomponent* comp = this->components[i].get();
			tz::assert(comp != nullptr);
			iresource* res = comp->get_resource();
			tz::assert(res != nullptr);
			if(res->get_access() == tz::gl::resource_access::static_fixed)
			{
				continue;
			}
			std::span<std::byte> underlying_component_data;
			switch(res->get_type())
			{
				case tz::gl::resource_type::buffer:
					underlying_component_data = static_cast<buffer_component_vulkan*>(comp)->vk_get_buffer().map_as<std::byte>();
				break;
				case tz::gl::resource_type::image:
					underlying_component_data = static_cast<image_component_vulkan*>(comp)->vk_get_image().map_as<std::byte>();
				break;
				default:
					tz::error("unrecognised resource_type");
				break;
			}
			auto resdata = res->data();
			tz::assert(resdata.size_bytes() <= underlying_component_data.size_bytes());
			std::copy(resdata.begin(), resdata.end(), underlying_component_data.begin());
			res->set_mapped_data(underlying_component_data);
		}
	}

	void renderer_resource_manager::populate_image_resource_views()
	{
		TZ_PROFZONE("render_resource_manager - populate image resourse views", 0xFFAAAA00);
		this->image_resource_views.clear();
		for(auto& component_ptr : this->components)
		{
			if(component_ptr->get_resource()->get_type() == tz::gl::resource_type::image)
			{
				vk2::Image& img = static_cast<image_component_vulkan*>(component_ptr.get())->vk_get_image();
				this->image_resource_views.emplace_back
				(
					vk2::ImageViewInfo
					{
						.image = &img,
						.aspect = vk2::derive_aspect_from_format(img.get_format()).front()
					}
				);
			}
			else
			{
				this->image_resource_views.push_back(vk2::ImageView::null());
			}
		}
	}

	void renderer_resource_manager::populate_image_resource_samplers()
	{
		TZ_PROFZONE("render_resource_manager - populate image resourse samplers", 0xFFAAAA00);
		this->image_resource_samplers.clear();
		for(auto& component_ptr : this->components)
		{
			if(component_ptr->get_resource()->get_type() == tz::gl::resource_type::image)
			{
				this->image_resource_samplers.emplace_back(renderer_resource_manager::make_fitting_sampler(*component_ptr->get_resource()));
			}
			else
			{
				this->image_resource_samplers.push_back(vk2::Sampler::null());
			}
		}
	}

	vk2::SamplerInfo renderer_resource_manager::make_fitting_sampler(const iresource& res)
	{
		tz::assert(res.get_type() == tz::gl::resource_type::image);
		vk2::LookupFilter filter = vk2::LookupFilter::Nearest;
		vk2::MipLookupFilter mip_filter = vk2::MipLookupFilter::Nearest;
		vk2::SamplerAddressMode mode = vk2::SamplerAddressMode::ClampToEdge;
#if TZ_DEBUG
		if(res.get_flags().contains({resource_flag::image_filter_nearest, resource_flag::image_filter_linear}))
		{
			tz::error("image_resource contained both resource_flags image_filter_nearest and image_filter_linear, which are mutually exclusive. Please submit a bug report.");
		}
#endif // TZ_DEBUG
		if(res.get_flags().contains(resource_flag::image_filter_nearest))
		{
			filter = vk2::LookupFilter::Nearest;
		}
		else if(res.get_flags().contains(resource_flag::image_filter_linear))
		{
			filter = vk2::LookupFilter::Linear;
		}

		if(res.get_flags().contains({resource_flag::image_wrap_clamp_edge, resource_flag::image_wrap_repeat, resource_flag::image_wrap_mirrored_repeat}))
		{
			tz::error("resource_flags included all 3 of image_wrap_clamp_edge, image_wrap_repeat and image_wrap_mirrored_repeat, all of which are mutually exclusive. Please submit a bug report.");
		}
		if(res.get_flags().contains(resource_flag::image_wrap_clamp_edge))
		{
			mode = vk2::SamplerAddressMode::ClampToEdge;
		}
		if(res.get_flags().contains(resource_flag::image_wrap_repeat))
		{
			mode = vk2::SamplerAddressMode::Repeat;
		}
		if(res.get_flags().contains(resource_flag::image_wrap_mirrored_repeat))
		{
			mode = vk2::SamplerAddressMode::MirroredRepeat;
		}
		
		return
		{
			.device = &tz::gl::get_device().vk_get_logical_device(),
			.min_filter = filter,
			.mag_filter = filter,
			.mipmap_mode = mip_filter,
			.address_mode_u = mode,
			.address_mode_v = mode,
			.address_mode_w = mode
		};
	}

	tz::maybe_owned_ptr<icomponent> renderer_resource_manager::make_component_from(iresource* resource)
	{
		TZ_PROFZONE("render_resource_manager - component factory", 0xFFAAAA00);
		tz::assert(resource != nullptr);
		switch(resource->get_type())
		{
			case tz::gl::resource_type::buffer:
				return tz::make_owned<buffer_component_vulkan>(*resource);
			break;
			case tz::gl::resource_type::image:
				return tz::make_owned<image_component_vulkan>(*resource);
			break;
		}
		return nullptr;
	}

//--------------------------------------------------------------------------------------------------

	renderer_descriptor_manager::renderer_descriptor_manager(const tz::gl::renderer_info& rinfo):
	renderer_resource_manager(rinfo)
	{
		TZ_PROFZONE("renderer_descriptor_manager - initialise", 0xFFAAAA00);
		this->deduce_descriptor_layout(rinfo.state());
		this->allocate_descriptors();
		this->write_descriptors(rinfo.state());
	}

	const vk2::DescriptorLayout& renderer_descriptor_manager::get_descriptor_layout() const
	{
		return this->descriptors.layout;
	}

	std::span<const vk2::DescriptorSet> renderer_descriptor_manager::get_descriptor_sets() const
	{
		return this->descriptors.data.sets;
	}

	bool renderer_descriptor_manager::empty() const
	{
		return this->descriptors.layout.descriptor_count() == 0;
	}

	void renderer_descriptor_manager::write_descriptors(const tz::gl::render_state& state)
	{
		TZ_PROFZONE("renderer_descriptor_manager - write descriptors ", 0xFFAAAA00);
		// early out if there's no descriptors to write to.
		if(this->empty())
		{
			tz::assert(this->descriptors.data.sets.empty());
			return;
		}
		// populate a list of buffer and image writes.
		// pair.first is the binding id, pair.second is the write info. 
		std::vector<std::pair<std::uint32_t, vk2::DescriptorSet::Write::BufferWriteInfo>> buffer_writes;
		buffer_writes.reserve(renderer_resource_manager::resource_count());
		std::vector<vk2::DescriptorSet::Write::ImageWriteInfo> image_writes;
		image_writes.reserve(renderer_resource_manager::resource_count());
		// not all buffers are going to be referenced by descriptors (e.g index and indirect buffers). we count them here and exclude them from the final descriptor counts.
		std::size_t uninterested_descriptor_count = 0;
		for(std::size_t i = 0; i < renderer_resource_manager::resource_count(); i++)
		{
			tz::gl::resource_handle rh = static_cast<tz::hanval>(i);
			// we dont want to expose index and draw indirect buffers to descriptors, so we skip them here.
			if(state.graphics.index_buffer == rh || state.graphics.draw_buffer == rh)
			{
				uninterested_descriptor_count++;			
				continue;
			}
			icomponent* comp = renderer_resource_manager::get_component(rh);
			tz::assert(comp != nullptr);
			iresource* res = comp->get_resource();
			tz::assert(res != nullptr);
			switch(res->get_type())
			{
				case tz::gl::resource_type::buffer:
				{
					vk2::Buffer& buf = static_cast<buffer_component_vulkan*>(comp)->vk_get_buffer();
					buffer_writes.push_back
					(std::make_pair(
		  				static_cast<std::uint32_t>(i - uninterested_descriptor_count),
		  				vk2::DescriptorSet::Write::BufferWriteInfo{
						.buffer = &buf,
						.buffer_offset = 0,
		  				.buffer_write_size = buf.size()
						}
					));
				}
				break;
				case tz::gl::resource_type::image:
					image_writes.push_back
					({
						.sampler = &renderer_resource_manager::get_image_resource_samplers()[i],
		  				.image_view = &renderer_resource_manager::get_image_resource_views()[i]
					});
				break;
				default:
					tz::error("invalid resource_type. memory corruption?");
				break;
			}
		}

		// now, apply buffer_writes and image_writes to all descriptor sets.
		const std::size_t frame_in_flight_count = tz::gl::get_device().get_swapchain().get_images().size();
		vk2::DescriptorPool::UpdateRequest update = tz::gl::get_device().vk_make_update_request(renderer_vulkan_base::uid);
		for(std::size_t i = 0; i < frame_in_flight_count; i++)
		{
			vk2::DescriptorSet& set = this->descriptors.data.sets[i];
			vk2::DescriptorSet::EditRequest req = set.make_edit_request();
			for(const auto& [binding_id, buf_write] : buffer_writes)
			{
				req.set_buffer(binding_id, buf_write);
			}
			if(!image_writes.empty())
			{
				for(std::size_t j = 0; j < image_writes.size(); j++)
				{
					req.set_image(buffer_writes.size(), image_writes[j], j);
				}
			}
			update.add_set_edit(req);
		}
		// tell the device to do all the writes.
		tz::gl::get_device().vk_update_sets(update, renderer_vulkan_base::uid);
	}

	void renderer_descriptor_manager::deduce_descriptor_layout(const tz::gl::render_state& state)
	{
		TZ_PROFZONE("renderer_descriptor_manager - deduce descriptor layout", 0xFFAAAA00);
		// figure out what descriptor layout we need, and set this->layout to it.
		// the old value of this->layout is discarded. it is an error to call this function while the layout is "in-use"
		std::size_t buffer_count = 0;
		std::size_t image_count = 0;
		for(std::size_t i = 0; i < renderer_resource_manager::resource_count(); i++)
		{
			const iresource* res = renderer_resource_manager::get_resource(static_cast<tz::hanval>(i));	
			tz::assert(res != nullptr);
			switch(res->get_type())
			{
				case tz::gl::resource_type::buffer:
					buffer_count++;
				break;
				case tz::gl::resource_type::image:
					image_count++;
				break;
			}
		}
		
		// now we know the number of buffers and resources, but we need a little bit of extra info.
		// what if one of the buffers is our index/draw-indirect buffer? if so, we need to exclude it from the descriptor layout, as it is not a shader resource.
		// this is why we need to know the render state.	
		if(state.graphics.index_buffer != tz::nullhand)
		{
			tz::assert(buffer_count > 0);
			buffer_count--;
		}
		if(state.graphics.draw_buffer != tz::nullhand)
		{
			tz::assert(buffer_count > 0);
			buffer_count--;
		}

		// now we can actually create the descriptor layout.
		vk2::DescriptorLayoutBuilder builder;
		builder.set_device(tz::gl::get_device().vk_get_logical_device());
		for(std::size_t i = 0; i < buffer_count; i++)
		{
			builder.with_binding
			({
				.type = vk2::DescriptorType::StorageBuffer,
				.count = 1,
				.flags =
				{
					vk2::DescriptorFlag::UpdateAfterBind,
					vk2::DescriptorFlag::UpdateUnusedWhilePending
				}
			});
		}
		if(image_count > 0)
		{
			builder.with_binding
			({
				.type = vk2::DescriptorType::ImageWithSampler,
				.count = static_cast<std::uint32_t>(image_count),
				.flags = 
				{
					vk2::DescriptorFlag::PartiallyBound,
					vk2::DescriptorFlag::UpdateAfterBind,
					vk2::DescriptorFlag::UpdateUnusedWhilePending
				}
			});
		}
		this->descriptors.layout = builder.build();
		if(buffer_count == 0 && image_count == 0)
		{
			tz::assert(this->empty());
		}
	}

	void renderer_descriptor_manager::allocate_descriptors()
	{
		TZ_PROFZONE("renderer_descriptor_manager - allocate descriptors ", 0xFFAAAA00);
		if(this->empty())
		{
			return;
		}
		if(!this->descriptors.data.sets.empty())
		{
			// we have a previous allocation.
			// todo: tell the device to free these descriptors.
			tz::error("support for handling previous descriptors allocation is NYI.");
		}
		// we need to know how many frames-in-flight we expect.
		tz::assert(!this->descriptors.layout.is_null());
		const std::size_t frame_in_flight_count = tz::gl::get_device().get_swapchain().get_images().size();
		tz::basic_list<const vk2::DescriptorLayout*> layouts;
		layouts.resize(frame_in_flight_count);
		for(std::size_t i = 0; i < frame_in_flight_count; i++)
		{
			layouts[i] = &this->descriptors.layout;
		}
		this->descriptors.data = tz::gl::get_device().vk_allocate_sets
		({
			.set_layouts = std::move(layouts)
		}, renderer_vulkan_base::uid);
		tz::assert(this->descriptors.data.success());
	}

//--------------------------------------------------------------------------------------------------

	renderer_output_manager::renderer_output_manager(const tz::gl::renderer_info& rinfo):
	renderer_descriptor_manager(rinfo),
	output(rinfo.get_output() == nullptr ? nullptr : rinfo.get_output()->unique_clone())
	{
		this->populate_render_targets(rinfo.get_options());
	}

	const ioutput* renderer_output_manager::get_output() const
	{
		return this->output.get();
	}

	ioutput* renderer_output_manager::get_output_mutable()
	{
		return this->output.get();
	}

	std::span<renderer_output_manager::render_target_t> renderer_output_manager::get_render_targets()
	{
		return this->render_targets;
	}

	tz::vec2ui renderer_output_manager::get_render_target_dimensions() const
	{
		tz::assert(this->render_targets.size(), "no render targets. logic error.");
		tz::assert(this->render_targets.front().colour_attachments.size(), "render target has no colour attachments. this isn't an error in of itself, but it is when trying to retrieve render target dimensions. should this method be improved to return depth dimensions (if a depth attachment exists) in this case? logic error.");
		return this->render_targets.front().colour_attachments.front().get_image().get_dimensions();
	}

	bool renderer_output_manager::targets_window() const
	{
		return this->get_output() == nullptr || this->get_output()->get_target() == output_target::window;
	}

	void renderer_output_manager::populate_render_targets(const tz::gl::renderer_options& options)
	{
		TZ_PROFZONE("renderer_output_manager - populate render targets", 0xFFAAAA00);
		this->render_targets.clear();
		auto& swapchain = tz::gl::get_device().get_swapchain();
		const std::uint32_t frame_in_flight_count = swapchain.get_images().size();
		this->render_targets.resize(frame_in_flight_count);
		for(std::size_t i = 0; i < frame_in_flight_count; i++)
		{
			auto& colours = this->render_targets[i].colour_attachments;
			if(this->targets_window())
			{
				// use swapchain images and device depth image.
				colours.push_back
				(vk2::ImageView{{
					.image = &swapchain.get_images()[i],
					.aspect = vk2::ImageAspectFlag::Colour
				 }});
				this->render_targets[i].depth_attachment = vk2::ImageView
				{{
					.image = &tz::gl::get_device().get_depth_image(),
					.aspect = vk2::ImageAspectFlag::Depth
				}};
			}
			else
			{
				// use image output.
				auto& imgout = static_cast<tz::gl::image_output&>(*this->output.get());
				// colour
				for(std::size_t j = 0; j < imgout.colour_attachment_count(); j++)
				{
					colours.push_back(vk2::ImageView
					{{
						.image = &imgout.get_colour_attachment(j).vk_get_image(),
						.aspect = vk2::ImageAspectFlag::Colour
					}});
				}
				// depth
				if(!options.contains(tz::gl::renderer_option::no_depth_testing))
				{
					// if we have depth enabled, we use the image output's depth image.
					// if it doesn't have one, we use the global device depth image.
					if(imgout.has_depth_attachment())
					{
						this->render_targets[i].depth_attachment = vk2::ImageView
						{{
							.image = &imgout.get_depth_attachment().vk_get_image(),
							.aspect = vk2::ImageAspectFlag::Depth
						}};
					}
					else
					{
						this->render_targets[i].depth_attachment = vk2::ImageView
						{{
							.image = &tz::gl::get_device().get_depth_image(),
							.aspect = vk2::ImageAspectFlag::Depth
						}};
					}
				}
			}
		}
	}

//--------------------------------------------------------------------------------------------------

	renderer_pipeline::renderer_pipeline(const tz::gl::renderer_info& rinfo):
	renderer_output_manager(rinfo)
	{
		TZ_PROFZONE("renderer_pipeline - initialise", 0xFFAAAA00);
		this->deduce_pipeline_config(rinfo);
		this->deduce_pipeline_layout();
		this->create_shader(rinfo);
		this->pipeline_cache = vk2::PipelineCache(tz::gl::get_device().vk_get_logical_device());
		this->update_pipeline(rinfo.state());
	}

	renderer_pipeline::pipeline_type_t renderer_pipeline::get_pipeline_type() const
	{
		return this->pipeline_config.type;
	}

	const vk2::Pipeline& renderer_pipeline::get_pipeline() const
	{
		return this->pipeline.data;
	}

	const vk2::PipelineLayout& renderer_pipeline::get_pipeline_layout() const
	{
		return this->pipeline.layout;
	}

	void renderer_pipeline::update_pipeline(const tz::gl::render_state& state)
	{
		TZ_PROFZONE("renderer_pipeline - update pipeline", 0xFFAAAA00);
		switch(this->get_pipeline_type())
		{
			case pipeline_type_t::graphics:
			{
				tz::assert(renderer_output_manager::get_render_targets().size(), "no render targets. please submit a bug report.");
				const render_target_t& render_target_info = renderer_output_manager::get_render_targets().front();
				tz::basic_list<vk2::ColourBlendState::AttachmentState> blending_options;
				vk2::ColourBlendState::AttachmentState blend_state = this->pipeline_config.alpha_blending ? vk2::ColourBlendState::alpha_blending() : vk2::ColourBlendState::no_blending();
				blending_options.resize(render_target_info.colour_attachments.size());
				std::fill(blending_options.begin(), blending_options.end(), blend_state);

				
				std::vector<VkFormat> colour_attachment_formats(render_target_info.colour_attachments.size());
				std::transform(render_target_info.colour_attachments.begin(), render_target_info.colour_attachments.end(), colour_attachment_formats.begin(),
				[](const vk2::ImageView& colour_view) -> VkFormat
				{
					return static_cast<VkFormat>(colour_view.get_image().get_format());
				});

				this->pipeline.data =
				{vk2::GraphicsPipelineInfo{
					.shaders = this->shader.native_data(),
					.state =
					{
						.viewport = vk2::create_basic_viewport(static_cast<tz::vec2>(renderer_output_manager::get_render_target_dimensions())),
						.rasteriser = {.polygon_mode = (state.graphics.wireframe_mode ? vk2::PolygonMode::Line : vk2::PolygonMode::Fill)},
						.depth_stencil =
						{
							.depth_testing = this->pipeline_config.depth_testing,
							.depth_writes = this->pipeline_config.depth_testing
						},
						.colour_blend =
						{
							.attachment_states = blending_options,
							.logical_operator = VK_LOGIC_OP_COPY
						},
						.dynamic =
						{
							.states = {vk2::DynamicStateType::Scissor}
						}
					},
					.pipeline_layout = &this->pipeline.layout,
					.render_pass = nullptr,
					.dynamic_rendering_state =
					{
						.colour_attachment_formats = colour_attachment_formats,
						.depth_format = render_target_info.depth_attachment.is_null() ? VK_FORMAT_UNDEFINED : static_cast<VkFormat>(render_target_info.depth_attachment.get_image().get_format())
					},
					.device = &tz::gl::get_device().vk_get_logical_device(),
				}, this->pipeline_cache};
			}
			break;
			case pipeline_type_t::compute:
				this->pipeline.data =
				{{
					.shader = this->shader.native_data(),
					.pipeline_layout = &this->pipeline.layout,
					.device = &tz::gl::get_device().vk_get_logical_device(),
				}, this->pipeline_cache};
			break;
		}
	}

	void renderer_pipeline::deduce_pipeline_config(const tz::gl::renderer_info& rinfo)
	{
		this->pipeline_config =
		{
			.depth_testing = !rinfo.get_options().contains(tz::gl::renderer_option::no_depth_testing),
			.alpha_blending = !rinfo.get_options().contains(tz::gl::renderer_option::alpha_blending),
			.type = rinfo.shader().has_shader(tz::gl::shader_stage::compute) ? pipeline_type_t::compute : pipeline_type_t::graphics
		};
		tz::assert(this->pipeline_config.valid);
	}

	void renderer_pipeline::create_shader(const tz::gl::renderer_info& rinfo)
	{
		TZ_PROFZONE("renderer_pipeline - create shader", 0xFFAAAA00);
		tz::basic_list<vk2::ShaderModuleInfo> modules;
		switch(this->get_pipeline_type())
		{
			case pipeline_type_t::graphics:
			{
				std::string_view vertex_source = rinfo.shader().get_shader(shader_stage::vertex);
				std::vector<char> vtx_src(vertex_source.length());
				std::copy(vertex_source.begin(), vertex_source.end(), vtx_src.begin());
				std::string_view fragment_source = rinfo.shader().get_shader(shader_stage::fragment);
				std::vector<char> frg_src(fragment_source.length());
				std::copy(fragment_source.begin(), fragment_source.end(), frg_src.begin());
				modules =
				{
					{
						.device = &tz::gl::get_device().vk_get_logical_device(),
						.type = vk2::ShaderType::vertex,
						.code = vtx_src
					},
					{
						.device = &tz::gl::get_device().vk_get_logical_device(),
						.type = vk2::ShaderType::fragment,
						.code = frg_src
					},
				};
				if(rinfo.shader().has_shader(shader_stage::tessellation_control) || rinfo.shader().has_shader(shader_stage::tessellation_evaluation))
				{
					std::string_view tesscon_source = rinfo.shader().get_shader(shader_stage::tessellation_control);
					std::vector<char> tessc_src(tesscon_source.length());
					std::copy(tesscon_source.begin(), tesscon_source.end(), tessc_src.begin());
					std::string_view tesseval_source = rinfo.shader().get_shader(shader_stage::tessellation_evaluation);
					std::vector<char> tesse_src(tesseval_source.length());
					std::copy(tesseval_source.begin(), tesseval_source.end(), tesse_src.begin());
					modules.add(vk2::ShaderModuleInfo
					{
						.device = &tz::gl::get_device().vk_get_logical_device(),
						.type = vk2::ShaderType::tessellation_control,
						.code = tessc_src
					});
					modules.add(vk2::ShaderModuleInfo
					{
						.device = &tz::gl::get_device().vk_get_logical_device(),
						.type = vk2::ShaderType::tessellation_evaluation,
						.code = tesse_src
					});
				}
			}
			break;
			case pipeline_type_t::compute:
			{
				std::string_view compute_source = rinfo.shader().get_shader(shader_stage::compute);
				std::vector<char> cmp_src(compute_source.length());
				std::copy(compute_source.begin(), compute_source.end(), cmp_src.begin());
				modules =
				{{
					.device = &tz::gl::get_device().vk_get_logical_device(),
					.type = vk2::ShaderType::compute,
					.code = cmp_src
				}};
			}
			break;
		}
		this->shader =
		{{
			.device = &tz::gl::get_device().vk_get_logical_device(),
			.modules = modules
		}};
	}

	void renderer_pipeline::deduce_pipeline_layout()
	{
		const volatile std::uint32_t frame_in_flight_count = tz::gl::get_device().get_swapchain().get_images().size();
		std::vector<const vk2::DescriptorLayout*> dlayouts(frame_in_flight_count, &renderer_descriptor_manager::get_descriptor_layout());
		this->pipeline.layout =
		{{
			.descriptor_layouts = std::move(dlayouts),
			.logical_device = &tz::gl::get_device().vk_get_logical_device(),
		}};
	}

//--------------------------------------------------------------------------------------------------

	renderer_command_processor::renderer_command_processor(const tz::gl::renderer_info& rinfo):
	renderer_pipeline(rinfo),
	render_wait_enabled(rinfo.get_options().contains(tz::gl::renderer_option::render_wait)),
	no_present_enabled(rinfo.get_options().contains(tz::gl::renderer_option::no_present)),
	render_wait_fence(vk2::FenceInfo{.device = &tz::gl::get_device().vk_get_logical_device()}),
	present_sync_semaphore(tz::gl::get_device().vk_get_logical_device())
	{
		TZ_PROFZONE("renderer_command_processor - initialise", 0xFFAAAA00);
		this->allocate_commands(command_type::both);
		this->scratch_initialise_static_resources();
		this->record_commands(rinfo.state(), rinfo.get_options(), rinfo.debug_get_name());
	}

	renderer_command_processor::~renderer_command_processor()
	{
		this->free_commands(command_type::both);
	}

	constexpr std::size_t cmdbuf_work_alloc_id = 0;
	constexpr std::size_t cmdbuf_scratch_alloc_id = 1;

	void renderer_command_processor::do_frame()
	{
		TZ_PROFZONE("renderer_command_processor - do frame", 0xFFAAAA00);
		const bool can_present = this->targets_window();
		const bool will_present = !this->no_present_enabled && can_present;
		const bool compute = renderer_pipeline::get_pipeline_type() == renderer_pipeline::pipeline_type_t::compute;
		tz::basic_list<const vk2::Semaphore*> extra_waits = {};
		tz::basic_list<const vk2::Semaphore*> extra_signals = {};
		// we potentially need to do a wait if our last frame id is still going.
		auto& dev = tz::gl::get_device();
		dev.vk_frame_wait(renderer_vulkan_base::uid);
		if(!compute && can_present)
		{
			// we need to wait on the image being available.
			const vk2::Semaphore& image_wait = dev.acquire_image(nullptr);
			if(will_present)
			{
				extra_waits.add(&image_wait);
				// we want our work to signal a semaphore which our present waits on.
				extra_signals.add(&this->present_sync_semaphore);
			}
		}
		// submit the work
		std::size_t frame_id = dev.vk_get_frame_id();
		if(!compute && can_present)
		{
			frame_id = dev.get_image_index();
		}
		vk2::Fence* signal_fence = nullptr;
		if(this->render_wait_enabled)
		{
			signal_fence = &this->render_wait_fence;
		}
		dev.vk_submit_command(renderer_vulkan_base::uid, cmdbuf_work_alloc_id, frame_id, std::span<const vk2::CommandBuffer>{&this->work_command_buffers()[frame_id], 1}, extra_waits, extra_signals, signal_fence);
		if(this->render_wait_enabled)
		{
			TZ_PROFZONE("renderer_command_processor - render wait", 0xFFAAAA00);
			this->render_wait_fence.wait_until_signalled();
			this->render_wait_fence.unsignal();
		}
		if(!compute && will_present)
		{
			// do the present.
			tz::basic_list<const vk2::BinarySemaphore*> present_waits{&this->present_sync_semaphore};
			dev.present_image(renderer_vulkan_base::uid, present_waits);
		}
	}

	void renderer_command_processor::do_scratch_work(std::function<void(vk2::CommandBufferRecording&)> record_commands)
	{
		TZ_PROFZONE("renderer_command_processor - scratch work...", 0xFFAAAA00);
		if(this->scratch_command_buffer().has_ever_recorded())
		{
			// this scratch command buffer is already populated with potentially different commands. purge and go again.
			this->allocate_commands(command_type::scratch);
		}
		vk2::CommandBuffer& buf = this->scratch_command_buffer();
		// firstly record the commands requested.
		{
			vk2::CommandBufferRecording rec = buf.record();
			record_commands(rec);
		}
		// then, execute them.
		constexpr std::size_t scratch_id = 1;
		tz::gl::get_device().vk_submit_and_run_commands_blocking(renderer_vulkan_base::uid, scratch_id, 0, buf);
	}

	void renderer_command_processor::set_work_commands(std::function<void(vk2::CommandBufferRecording&, unsigned int)> work_record_commands)
	{
		TZ_PROFZONE("renderer_command_processor - set work commands", 0xFFAAAA00);
		auto bufs = this->work_command_buffers();
		if(std::any_of(bufs.begin(), bufs.end(), [](const auto& work_buf)
		{
			tz::assert(!work_buf.is_recording(), "attempt to set work commands while one or more of them is recording. sync error. please submit a bug report.");
			return work_buf.command_count() > 0;
		}))
		{
			// if any of the work command buffers have already been recorded, we need to purge.
			this->allocate_commands(command_type::work);
			// re-assign as we have new bufs now.
			bufs = this->work_command_buffers();
		}
		for(unsigned int i = 0; i < bufs.size(); i++)
		{
			vk2::CommandBufferRecording record = bufs[i].record();
			work_record_commands(record, i);
		}
	}

	void renderer_command_processor::record_commands(const tz::gl::render_state& state, const tz::gl::renderer_options& options, std::string label)
	{
		TZ_PROFZONE("renderer_command_processor - record commands", 0xFFAAAA00);
		switch(renderer_pipeline::get_pipeline_type())
		{
			case renderer_pipeline::pipeline_type_t::graphics:
				this->record_render_commands(state, options, label);
			break;
			case renderer_pipeline::pipeline_type_t::compute:
				this->record_compute_commands(state, options, label);
			break;
			default:
				tz::error("invalid value for pipeline type. memory corruption?");
			break;
		}
	}

	void renderer_command_processor::scratch_initialise_static_resources()
	{
		TZ_PROFZONE("renderer_command_processor - scratch initialise static resources", 0xFFAAAA00);
		// we need to create a staging buffer for each static_fixed buffer and image resource we have.
		// these staging buffers should be cpu-writable (like a dynamic-fixed buffer resource), and we write in the data in the mapped ptr just like a dynamic resource.
		// afterwards, we record a scratch buffer which does transfers between each staging buffer and its corresponding static resource.
		// we then instantly wait on this work to be complete.
		std::vector<vk2::Buffer> resource_staging_buffers;
		resource_staging_buffers.reserve(renderer_resource_manager::resource_count());
		for(std::size_t i = 0; i < renderer_resource_manager::resource_count(); i++)
		{
			const icomponent* cmp = renderer_resource_manager::get_component(static_cast<tz::hanval>(i));
			tz::assert(cmp != nullptr);
			const iresource* res = cmp->get_resource();
			tz::assert(res != nullptr);
			if(res->get_access() != tz::gl::resource_access::static_fixed)
			{
				resource_staging_buffers.push_back(vk2::Buffer::null());
				continue;
			}
			switch(res->get_type())
			{
				// create staging buffers and copy over spans.
				case tz::gl::resource_type::buffer:
					resource_staging_buffers.push_back
					({{
						.device = &tz::gl::get_device().vk_get_logical_device(),
		  				.size_bytes = res->data().size_bytes(),
		  				.usage = {vk2::BufferUsage::TransferSource},
		  				.residency = vk2::MemoryResidency::CPU
					}});
				break;
				case tz::gl::resource_type::image:
				{
					auto* img = static_cast<const image_component_vulkan*>(cmp);
					tz::assert(img != nullptr);
					resource_staging_buffers.push_back
					({{
						.device = &tz::gl::get_device().vk_get_logical_device(),
		  				.size_bytes = tz::gl::pixel_size_bytes(img->get_format()) * img->get_dimensions()[0] * img->get_dimensions()[1],
		  				.usage = {vk2::BufferUsage::TransferSource},
		  				.residency = vk2::MemoryResidency::CPU
					}});
				}
				break;
				default:
					tz::error("unrecognised resource_type. memory corruption?");
				break;
			}
			// get the new staging buffer
			vk2::Buffer& new_buf = resource_staging_buffers.back();
			// write resource data
			{
				void* mapped_ptr = new_buf.map();
				std::memcpy(mapped_ptr, res->data().data(), res->data().size_bytes());
				new_buf.unmap();
			}
		}
		this->do_static_resource_transfers(resource_staging_buffers);
	}

	void renderer_command_processor::queue_resource_write(tz::gl::renderer_edit::resource_write rwrite)
	{
		iresource* res = renderer_resource_manager::get_resource(rwrite.resource);
		tz::assert(res != nullptr);
		if(res->get_access() != tz::gl::resource_access::static_fixed)
		{
			// if this is a dynamic resource, we can trivially do the write now.
			tz::assert(res->data().size_bytes() >= (rwrite.offset + rwrite.data.size_bytes()), "resource write too long. resource is %zu bytes, write was (%zu + %zu) bytes", res->data().size_bytes(), rwrite.offset, rwrite.data.size_bytes());
			std::memcpy(reinterpret_cast<char*>(res->data().data()) + rwrite.offset, rwrite.data.data(), rwrite.data.size_bytes());
			return;
		}
		auto id = static_cast<std::size_t>(static_cast<tz::hanval>(rwrite.resource));
		// get the buffer representing the staging buffer for this resource.
		tz::assert(id < this->pending_resource_write_staging_buffers.size());
		vk2::Buffer& buf = this->pending_resource_write_staging_buffers[id];
		// initialise it.
		buf = vk2::Buffer
		{{
			.device = &tz::gl::get_device().vk_get_logical_device(),
			.size_bytes = rwrite.data.size_bytes(),
			.usage = {vk2::BufferUsage::TransferSource},
			.residency = vk2::MemoryResidency::CPU
		}};
		// write the resource write data.
		{
			void* mapped_ptr = buf.map();
			std::memcpy(static_cast<char*>(mapped_ptr) + rwrite.offset, rwrite.data.data(), rwrite.data.size_bytes());
			buf.unmap();
		}	
		// pending changes are now resident in that cpu buffer. submit resource writes will send off the work to be done.
	}

	void renderer_command_processor::submit_resource_writes()
	{
		this->do_static_resource_transfers(this->pending_resource_write_staging_buffers);
		this->reset_resource_write_buffers();
	}

	void renderer_command_processor::reset_resource_write_buffers()
	{
		this->pending_resource_write_staging_buffers.clear();
		for(std::size_t i = 0; i < renderer_resource_manager::resource_count(); i++)
		{
			this->pending_resource_write_staging_buffers.push_back(vk2::Buffer::null());
		}
	}

	void renderer_command_processor::record_render_commands(const tz::gl::render_state& state, const tz::gl::renderer_options& options, std::string label)
	{
		TZ_PROFZONE("renderer_command_processor - record render commands", 0xFFAAAA00);
		this->set_work_commands([this, state, &label, &options](vk2::CommandBufferRecording& record, unsigned int render_target_id)
		{
			const bool present = !options.contains(tz::gl::renderer_option::no_present) && renderer_output_manager::targets_window();
			record.debug_begin_label({.name = label});
			vk2::Image& cur_swapchain_image = tz::gl::get_device().get_swapchain().get_images()[render_target_id];

			// if 'no clear output' isn't specified, then we can say old layout is undefined, so we're guaranteed to be in colour attachment after.
			// after a present, we want this to be the case so this should work?
			if(!options.contains(tz::gl::renderer_option::no_clear_output) && renderer_output_manager::targets_window())
			{
				record.transition_image_layout
				(vk2::VulkanCommand::TransitionImageLayout{
					.image = &cur_swapchain_image,
					.target_layout = vk2::ImageLayout::ColourAttachment,
					.source_access = {},
					.destination_access = {vk2::AccessFlag::ColourAttachmentWrite},
					.source_stage = vk2::PipelineStage::Bottom,
					.destination_stage = vk2::PipelineStage::ColourAttachmentOutput,
					.image_aspects = {vk2::ImageAspectFlag::Colour}
				});
			}

			// dynamic rendering { bind pipeline -> bind descriptor set -> set scissor -> draw }
			{
				renderer_output_manager::render_target_t& render_target = renderer_output_manager::get_render_targets()[render_target_id];
				vk2::ImageView* depth = &render_target.depth_attachment;
				if(depth->is_null())
				{
					depth = nullptr;
				}
				vk2::CommandBufferRecording::DynamicRenderingRun run{record, render_target.colour_attachments, depth,
				{
					.clear_colour = state.graphics.clear_colour,
					.colour_load = options.contains(tz::gl::renderer_option::no_clear_output) ? vk2::LoadOp::Load : vk2::LoadOp::Clear,
					.colour_store = vk2::StoreOp::Store,
					.depth_load = options.contains(tz::gl::renderer_option::no_clear_output) ? vk2::LoadOp::Load : vk2::LoadOp::Clear,
					.depth_store = options.contains(tz::gl::renderer_option::no_present) ? vk2::StoreOp::Store : vk2::StoreOp::DontCare
				}};
				record.bind_pipeline
				({
					.pipeline = &renderer_pipeline::get_pipeline()
				});
				if(!renderer_descriptor_manager::empty())
				{
					tz::basic_list<const vk2::DescriptorSet*> sets;
					sets = {&renderer_descriptor_manager::get_descriptor_sets()[render_target_id]};
					record.bind_descriptor_sets
					(vk2::VulkanCommand::BindDescriptorSets{
						.pipeline_layout = &renderer_pipeline::get_pipeline_layout(),
		  				.context = vk2::PipelineContext::graphics,
						.descriptor_sets = sets,
		  				.first_set_id = 0u
					});
				}
				{
					// scissor
					tz::vec2ui offset{0u, 0u};
					tz::vec2ui extent = renderer_output_manager::get_render_target_dimensions();
					const ioutput* output = renderer_output_manager::get_output();
					if(output != nullptr && output->scissor != scissor_region::null())
					{
						offset = output->scissor.offset;
						extent = output->scissor.extent;
					}
					record.set_scissor_dynamic
					({
						.offset = offset,
						.extent = extent
					});
				}

				auto* index_buffer = static_cast<tz::gl::buffer_component_vulkan*>(renderer_resource_manager::get_component(state.graphics.index_buffer));
				auto* indirect_buffer = static_cast<tz::gl::buffer_component_vulkan*>(renderer_resource_manager::get_component(state.graphics.draw_buffer));
				if(index_buffer == nullptr)
				{
					if(indirect_buffer == nullptr)
					{
						record.draw
						({
							.vertex_count = static_cast<std::uint32_t>(3 * state.graphics.tri_count),
							.instance_count = 1,
							.first_vertex = 0,
							.first_instance = 0
						});
					}
					else
					{
						if(options.contains(tz::gl::renderer_option::draw_indirect_count))
						{
							record.draw_indirect_count
							({
								.draw_indirect_buffer = &indirect_buffer->vk_get_buffer(),
								.max_draw_count = static_cast<std::uint32_t>(indirect_buffer->get_resource()->data().size_bytes() / sizeof(VkDrawIndirectCommand)),
								.stride = static_cast<std::uint32_t>(sizeof(VkDrawIndirectCommand))
							});
						}
						else
						{
							record.draw_indirect
							({
								.draw_indirect_buffer = &indirect_buffer->vk_get_buffer(),
								.draw_count = static_cast<std::uint32_t>(indirect_buffer->get_resource()->data().size_bytes() / sizeof(VkDrawIndirectCommand)),
								.stride = static_cast<std::uint32_t>(sizeof(VkDrawIndirectCommand))
							});
						}
					}
				}
				else
				{
					record.bind_index_buffer({.index_buffer = &index_buffer->vk_get_buffer()});
					if(indirect_buffer == nullptr)
					{
						record.draw_indexed
						({
							.index_count = static_cast<std::uint32_t>(3 * state.graphics.tri_count)
						});
					}
					else
					{
						if(options.contains(tz::gl::renderer_option::draw_indirect_count))
						{
							tz::error("draw_indirect_count indexed is NYI");
						}
						else
						{
							record.draw_indexed_indirect
							({
								.draw_indirect_buffer = &indirect_buffer->vk_get_buffer(),
								.draw_count = static_cast<std::uint32_t>(indirect_buffer->get_resource()->data().size_bytes() / sizeof(VkDrawIndexedIndirectCommand)),
								.stride = static_cast<std::uint32_t>(sizeof(VkDrawIndexedIndirectCommand)),
								.offset = static_cast<VkDeviceSize>(0)
							});
						}
					}
				}
			}
			if(present)
			{
				record.transition_image_layout
				(vk2::VulkanCommand::TransitionImageLayout{
					.image = &cur_swapchain_image,
					.target_layout = vk2::ImageLayout::Present,
		 			.old_layout = vk2::ImageLayout::ColourAttachment,
					.source_access = {vk2::AccessFlag::ColourAttachmentWrite},
					.destination_access = {},
					.source_stage = vk2::PipelineStage::ColourAttachmentOutput,
					.destination_stage = vk2::PipelineStage::Top,
					.image_aspects = {vk2::ImageAspectFlag::Colour}
				});
			}
			record.debug_end_label({});
		});
	}

	void renderer_command_processor::record_compute_commands(const tz::gl::render_state& state, const tz::gl::renderer_options& options, std::string label)
	{
		TZ_PROFZONE("renderer_command_processor - record compute commands", 0xFFAAAA00);
		(void)options;
		this->set_work_commands([this, state, &label](vk2::CommandBufferRecording& record, unsigned int render_target_id)
		{
			record.debug_begin_label({.name = label});
			// bind pipeline -> bind descriptor set -> dispatch
			record.bind_pipeline
			({
				.pipeline = &renderer_pipeline::get_pipeline()
			});
			if(!renderer_descriptor_manager::empty())
			{
				tz::basic_list<const vk2::DescriptorSet*> sets;
				sets = {&renderer_descriptor_manager::get_descriptor_sets()[render_target_id]};
				record.bind_descriptor_sets
				(vk2::VulkanCommand::BindDescriptorSets{
					.pipeline_layout = &renderer_pipeline::get_pipeline_layout(),
					.context = vk2::PipelineContext::compute,
					.descriptor_sets = sets,
					.first_set_id = 0u
				});
			}
			record.dispatch
			({
				.groups = state.compute.kernel
			});
			record.debug_end_label({});
		});
	}

	void renderer_command_processor::allocate_commands(renderer_command_processor::command_type t)
	{
		TZ_PROFZONE("renderer_command_processor - allocate commands", 0xFFAAAA00);
		if(this->command_allocations.size() > 0 /* this might be incorrect logic */)
		{
			this->free_commands(t);
			if(t == renderer_command_processor::command_type::work)
			{
				// now the only remaining element is the scratch commands. need to push_front as work commands should go first.
				this->command_allocations.insert(this->command_allocations.begin(), vk2::CommandPool::AllocationResult{});
			}
			// else (scratch or both), meaning we're already in the correct order (work commands first), so no need to do anything.
		}
		this->command_allocations.resize(2);
		const std::uint32_t frame_in_flight_count = tz::gl::get_device().get_swapchain().get_images().size();
		const std::size_t scratch_buffer_count = 1;
		// we want a quantity of command buffers equal to `frame_in_flight_count` + 1.
		// the +1 is a scratch buffer which will be used for various commands (mainly renderer edits, but a few things in setup aswell, such as static resource writes).

		// tell the command pool about our intentions.
		auto& dev = tz::gl::get_device();
		dev.vk_command_pool_touch(renderer_vulkan_base::uid,
		{
			// todo: no magic values.
			.compute = false,
			.requires_present = true
		});
		// one allocation for our render/compute work commands.
		if(t == renderer_command_processor::command_type::work || t == renderer_command_processor::command_type::both)
		{
			this->command_allocations[cmdbuf_work_alloc_id] = dev.vk_allocate_commands
			({
				.buffer_count = frame_in_flight_count
			}, renderer_vulkan_base::uid);
		}
		
		if(t == renderer_command_processor::command_type::scratch || t == renderer_command_processor::command_type::both)
		{
			this->command_allocations[cmdbuf_scratch_alloc_id] = dev.vk_allocate_commands
			({
				.buffer_count = scratch_buffer_count
			}, renderer_vulkan_base::uid);
		}
	}

	void renderer_command_processor::free_commands(renderer_command_processor::command_type t)
	{
		TZ_PROFZONE("renderer_command_processor - free commands", 0xFFAAAA00);
		if(this->command_allocations.empty())
		{
			// early out if there's nothing to free.
			return;
		}
		if(t == command_type::scratch || t == command_type::both)
		{
			tz::gl::get_device().vk_free_commands(renderer_vulkan_base::uid, cmdbuf_scratch_alloc_id, this->command_allocations[cmdbuf_scratch_alloc_id].buffers);
			this->command_allocations.pop_back();
		}
		if(t == command_type::work || t == command_type::both)
		{
			tz::gl::get_device().vk_free_commands(renderer_vulkan_base::uid, cmdbuf_work_alloc_id, this->command_allocations[cmdbuf_work_alloc_id].buffers);
			// pop_front() doesnt exist in std::vector ugh
			this->command_allocations.erase(this->command_allocations.begin());
		}
		if(t == command_type::both)
		{
			tz::assert(this->command_allocations.empty());
		}
	}

	void renderer_command_processor::do_static_resource_transfers(std::span<vk2::Buffer> resource_staging_buffers)
	{
		TZ_PROFZONE("renderer_command_processor - do static resource transfers", 0xFFAAAA00);
		tz::assert(resource_staging_buffers.size() == renderer_resource_manager::resource_count(), "unexpected number of resource staging buffers. expected %u, got %zu", renderer_resource_manager::resource_count(), resource_staging_buffers.size());
		this->do_scratch_work([this, &resource_staging_buffers](vk2::CommandBufferRecording& recording)
		{
			// we now have all the staging buffers we need. do all necessary transfers.
			for(std::size_t i = 0; i < resource_staging_buffers.size(); i++)
			{
				vk2::Buffer& staging_buffer = resource_staging_buffers[i];
				tz::gl::icomponent* cmp = renderer_resource_manager::get_component(static_cast<tz::hanval>(i));
				const tz::gl::iresource* res = cmp->get_resource();
				if(staging_buffer.is_null())
				{
					continue;
				}
				tz::assert(res->get_access() == tz::gl::resource_access::static_fixed, "while initialising static resources, detected non-static-fixed resource at handle %zu that somehow ended up with a non-null staging buffer. logic error. please submit a bug report.", i);
				switch(res->get_type())
				{
					case tz::gl::resource_type::buffer:
						// super easy, just copy the buffer.
						recording.buffer_copy_buffer
						({
							.src = &staging_buffer,
		   					.dst = &static_cast<buffer_component_vulkan*>(cmp)->vk_get_buffer()
						});
					break;
					case tz::gl::resource_type::image:
					{
						// not so easy. copy the buffer, but do some layout transitions first.
						vk2::Image& img = static_cast<image_component_vulkan*>(cmp)->vk_get_image();
						vk2::ImageAspectFlags aspect = vk2::derive_aspect_from_format(img.get_format());
						// Image will initially be in undefined layout. We need to:
						// - Transition the texture component to TransferDestination
						// - Transfer from the staging texture buffer
						// - Transition the texture component to ShaderResource so it can be used in the shader.
						recording.transition_image_layout
						({
							.image = &img,
							.target_layout = vk2::ImageLayout::TransferDestination,
							.source_access = {vk2::AccessFlag::NoneNeeded},
							.destination_access = {vk2::AccessFlag::TransferOperationWrite},
							.source_stage = vk2::PipelineStage::Bottom,
							.destination_stage = vk2::PipelineStage::TransferCommands,
							.image_aspects = aspect
						});
						recording.buffer_copy_image
						({
							.src = &staging_buffer,
							.dst = &img,
							.image_aspects = aspect
						});
						recording.transition_image_layout
						({
							.image = &img,
							.target_layout = vk2::ImageLayout::ShaderResource,
							.source_access = {vk2::AccessFlag::TransferOperationWrite},
							.destination_access = {vk2::AccessFlag::ShaderResourceRead},
							.source_stage = vk2::PipelineStage::TransferCommands,
							.destination_stage = vk2::PipelineStage::FragmentShader,
							.image_aspects = aspect
						});
					}
					break;
					default:
						tz::error("invalid resource_type. memory corruption?");
					break;
				}
			}
		});
	}

	std::span<vk2::CommandBuffer> renderer_command_processor::work_command_buffers()
	{
		return this->command_allocations[cmdbuf_work_alloc_id].buffers;
	}

	vk2::CommandBuffer& renderer_command_processor::scratch_command_buffer()
	{
		tz::assert(this->command_allocations[cmdbuf_scratch_alloc_id].buffers.length() == 1, "detected more than 1 scratch command. engine dev - you forgot to refactor `scratch_command_buffer()`. please submit a bug report.");
		return this->command_allocations[cmdbuf_scratch_alloc_id].buffers.front();
	}

//--------------------------------------------------------------------------------------------------

	renderer_vulkan2::renderer_vulkan2(const tz::gl::renderer_info& rinfo):
	renderer_command_processor(rinfo),
	options(rinfo.get_options()),
	state(rinfo.state()),
	window_cache_dims(tz::window().get_dimensions()),
	null_flag(false),
	debug_name(rinfo.debug_get_name())
	{
		TZ_PROFZONE("renderer_vulkan2 - initialise", 0xFFAAAA00);
		renderer_command_processor::reset_resource_write_buffers();
	}

	renderer_vulkan2::~renderer_vulkan2()
	{
		tz::gl::get_device().vk_get_logical_device().wait_until_idle();
	}

	const tz::gl::renderer_options& renderer_vulkan2::get_options() const
	{
		return this->options;
	}

	const tz::gl::render_state& renderer_vulkan2::get_state() const
	{
		return this->state;
	}

	void renderer_vulkan2::render()
	{
		TZ_PROFZONE("renderer_vulkan2 - render", 0xFFAAAA00);
		if(!this->check_and_handle_resize())
		{
			return;
		}
		renderer_command_processor::do_frame();
	}

	template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
	template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

	void renderer_vulkan2::edit(tz::gl::renderer_edit_request req)
	{
		TZ_PROFZONE("renderer_vulkan2 - edit", 0xFFAAAA00);
		this->check_and_handle_resize();
		if(req.empty())
		{
			return;
		}
		tz::gl::get_device().vk_get_logical_device().wait_until_idle();
		edit_side_effects side_effects = {};
		bool resource_writes = false;
		for(const auto& edit : req)
		{
			std::visit(overloaded{
				// BUFFER RESIZE
				[&side_effects, this](tz::gl::renderer_edit::buffer_resize arg)
				{
					auto bufcomp = static_cast<buffer_component_vulkan*>(renderer_resource_manager::get_component(arg.buffer_handle));
					if(bufcomp->size() != arg.size)
					{
						bufcomp->resize(arg.size);
						if(arg.buffer_handle == this->state.graphics.index_buffer || arg.buffer_handle == this->state.graphics.draw_buffer)
						{
							  side_effects.rerecord_work_commands = true;
						}
						else
						{
							side_effects.rewrite_buffer_descriptors = true;
						}
					}
				},
				// IMAGE RESIZE
				[&side_effects, this](tz::gl::renderer_edit::image_resize arg)
				{
					auto imgcomp = static_cast<image_component_vulkan*>(renderer_resource_manager::get_component(arg.image_handle));
					if(imgcomp->get_dimensions() != arg.dimensions)
					{
						imgcomp->resize(arg.dimensions);
						renderer_resource_manager::notify_image_dirty(arg.image_handle);
						side_effects.rewrite_image_descriptors = true;
					}
				},
				// RESOURCE WRITE
				[&resource_writes, &side_effects, this](tz::gl::renderer_edit::resource_write arg)
				{
					renderer_command_processor::queue_resource_write(arg);
					resource_writes = true;
				},
				[&side_effects, this](tz::gl::renderer_edit::resource_reference arg)
				{
					tz::assert(arg.component != nullptr && arg.component->get_resource() != nullptr);
					tz::assert(static_cast<std::size_t>(static_cast<tz::hanval>(arg.resource)) < renderer_resource_manager::resource_count(), "resource_write resource handle was invalid.");
					tz::assert(arg.component->get_resource()->get_type() == renderer_resource_manager::get_resource(arg.resource)->get_type(), "resource_write component refers to a resource type that doesn't match the existing resource associated with the given handle.");
					renderer_resource_manager::reseat_resource(arg);
					// we will need to rewrite the associated descriptor. horribly heavy-handed to rewrite all of them, but that's what we do for now.
					// TODO?: write more code to support updating only a single descriptor.
					switch(arg.component->get_resource()->get_type())
					{
						case tz::gl::resource_type::buffer:
							side_effects.rewrite_buffer_descriptors = true;
						break;
						case tz::gl::resource_type::image:
							side_effects.rewrite_image_descriptors = true;
						break;
					}
				},
				[&side_effects, this](tz::gl::renderer_edit::scissor arg)
				{
					ioutput* out = renderer_output_manager::get_output_mutable();
					tz::assert(out != nullptr);
					if(arg.offset != out->scissor.offset || arg.extent != out->scissor.extent)
					{
						out->scissor.offset = arg.offset;
						out->scissor.extent = arg.extent;
						side_effects.rerecord_work_commands = true;
					}
				},
				// COMPUTE CONFIG
				[&side_effects, this](tz::gl::renderer_edit::compute_config arg)
				{
					if(arg.kernel != state.compute.kernel)
					{
						state.compute.kernel = arg.kernel;
						if(renderer_pipeline::get_pipeline_type() == pipeline_type_t::compute)
						{
							side_effects.rerecord_work_commands = true;
						}
						else
						{
							tz::report("Detected compute config renderer edit, but the renderer is not a compute renderer. Ignoring...");
						}
					}
				},
				[&side_effects, this](tz::gl::renderer_edit::render_config arg)
				{
					const bool valid = renderer_pipeline::get_pipeline_type() == pipeline_type_t::graphics;
					if(!valid)
					{
						tz::report("Detected render config renderer edit, but the renderer is a compute renderer. Ignoring...");
					}
					if(arg.wireframe_mode.has_value() && arg.wireframe_mode.value() != state.graphics.wireframe_mode)
					{
						state.graphics.wireframe_mode = arg.wireframe_mode.value();
						if(valid)
						{
							side_effects.recreate_pipeline = true;
						}
					}
					if(arg.clear_colour.has_value() && arg.clear_colour.value() != state.graphics.clear_colour)
					{
						state.graphics.clear_colour = arg.clear_colour.value();
						if(valid)
						{
							side_effects.recreate_pipeline = true;
						}
					}
					if(arg.tri_count.has_value() && arg.tri_count.value() != state.graphics.tri_count)
					{
						state.graphics.tri_count = arg.tri_count.value();
						if(valid)
						{
							side_effects.rerecord_work_commands = true;
						}
					}
				},
				// UNKNOWN
				[](auto arg)
				{
					(void)arg;
					tz::error("renderer edit type NYFI");
				}
			}, edit);
		}

		if(resource_writes)
		{
			renderer_command_processor::submit_resource_writes();
		}

		if(side_effects.rewrite_buffer_descriptors || side_effects.rewrite_image_descriptors)
		{
			renderer_descriptor_manager::write_descriptors(this->state);
		}
		if(side_effects.recreate_pipeline)
		{
			renderer_pipeline::update_pipeline(this->state);
			side_effects.rerecord_work_commands = true;
		}
		if(side_effects.rerecord_work_commands)
		{
			renderer_command_processor::record_commands(this->state, this->options, this->debug_name);
		}
		if(side_effects.rewrite_static_resources)
		{
			renderer_command_processor::scratch_initialise_static_resources();
		}
	}

	void renderer_vulkan2::dbgui()
	{
		tz::gl::common_renderer_dbgui(*this);
	}

	std::string_view renderer_vulkan2::debug_get_name() const
	{
		return this->debug_name;
	}

	renderer_vulkan2 renderer_vulkan2::null()
	{
		return {};
	}

	bool renderer_vulkan2::is_null() const
	{
		return this->null_flag;
	}

	bool renderer_vulkan2::check_and_handle_resize()
	{
		if(tz::window().get_dimensions() == tz::vec2ui::zero())
		{
			TZ_PROFZONE("renderer_vulkan2 - wait while minimised", 0xFFAAAA00);
			tz::wsi::wait_for_event();
			return false;
		}
		if(renderer_output_manager::targets_window() && (this->window_cache_dims != tz::window().get_dimensions()))
		{
			this->do_resize();
			this->window_cache_dims = tz::window().get_dimensions();
		}
		return true;
	}

	void renderer_vulkan2::do_resize()
	{
		TZ_PROFZONE("renderer_vulkan2 - do resize", 0xFFAAAA00);
		tz::gl::get_device().vk_notify_resize();
		for(std::size_t i = 0; i < this->resource_count(); i++)
		{
			tz::gl::resource_handle rh{static_cast<tz::hanval>(i)};
			icomponent* cmp = this->get_component(rh);
			iresource* res = cmp->get_resource();
			if(res->get_type() == tz::gl::resource_type::image && res->get_flags().contains(tz::gl::resource_flag::renderer_output))
			{
				static_cast<image_component_vulkan*>(cmp)->resize(tz::window().get_dimensions());
				renderer_resource_manager::notify_image_dirty(rh);
			}
		}
		renderer_output_manager::populate_render_targets(this->options);
		renderer_pipeline::update_pipeline(this->state);
		renderer_command_processor::record_commands(this->state, this->options, this->debug_name);
	}
}
#endif // TZ_VULKAN
