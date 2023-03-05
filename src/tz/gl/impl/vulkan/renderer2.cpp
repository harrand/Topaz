#include "tz/gl/impl/vulkan/renderer2.hpp"
#include "tz/core/profile.hpp"
#include "tz/gl/device.hpp"

namespace tz::gl
{
	unsigned int renderer_vulkan_base::uid_counter = 0;
//--------------------------------------------------------------------------------------------------
	renderer_resource_manager::renderer_resource_manager(const tz::gl::renderer_info& rinfo):
	AssetStorageCommon<iresource>(rinfo.get_resources())
	{
		TZ_PROFZONE("render_resource_manager - initialise", 0xFFAAAA00);
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
		return this->components[static_cast<std::size_t>(static_cast<tz::hanval>(rh))].get();
	}

	icomponent* renderer_resource_manager::get_component(tz::gl::resource_handle rh)
	{
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
			.device = &tz::gl::get_device2().vk_get_logical_device(),
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
	}

	bool renderer_descriptor_manager::empty() const
	{
		return this->layout.is_null();
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
		if(buffer_count == 0 && image_count == 0)
		{
			// we have no shader resources -- this we should be marked as empty.
			this->layout = vk2::DescriptorLayout::null();
			// and then early-out because we don't need to do anymore work.
			return;
		}

		// now we can actually create the descriptor layout.
		vk2::DescriptorLayoutBuilder builder;
		builder.set_device(tz::gl::get_device2().vk_get_logical_device());
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
		this->layout = builder.build();
	}

	void renderer_descriptor_manager::allocate_descriptors()
	{
		TZ_PROFZONE("renderer_descriptor_manager - allocate descriptors ", 0xFFAAAA00);
		if(this->empty())
		{
			return;
		}
		if(!this->descriptors.sets.empty())
		{
			// we have a previous allocation.
			// todo: tell the device to free these descriptors.
			tz::error("support for handling previous descriptors allocation is NYI.");
		}
		// we need to know how many frames-in-flight we expect.
		tz::assert(!this->layout.is_null());
		const std::size_t frame_in_flight_count = tz::gl::get_device2().get_swapchain().get_images().size();
		tz::basic_list<const vk2::DescriptorLayout*> layouts;
		layouts.resize(frame_in_flight_count);
		for(std::size_t i = 0; i < frame_in_flight_count; i++)
		{
			layouts[i] = &this->layout;
		}
		this->descriptors = tz::gl::get_device2().vk_allocate_sets
		({
			.set_layouts = std::move(layouts)
		}, renderer_vulkan_base::uid);
		tz::assert(this->descriptors.success());
	}

	void renderer_descriptor_manager::write_descriptors(const tz::gl::render_state& state)
	{
		TZ_PROFZONE("renderer_descriptor_manager - write descriptors ", 0xFFAAAA00);
		// early out if there's no descriptors to write to.
		if(this->empty())
		{
			tz::assert(this->descriptors.sets.empty());
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
		const std::size_t frame_in_flight_count = tz::gl::get_device2().get_swapchain().get_images().size();
		vk2::DescriptorPool::UpdateRequest update = tz::gl::get_device2().vk_make_update_request(renderer_vulkan_base::uid);
		for(std::size_t i = 0; i < frame_in_flight_count; i++)
		{
			vk2::DescriptorSet& set = this->descriptors.sets[i];
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
		tz::gl::get_device2().vk_update_sets(update, renderer_vulkan_base::uid);
	}
//--------------------------------------------------------------------------------------------------

	renderer_output_manager::renderer_output_manager(const tz::gl::renderer_info& rinfo):
	renderer_descriptor_manager(rinfo),
	output(rinfo.get_output())
	{
		this->populate_render_targets();
	}

	void renderer_output_manager::populate_render_targets()
	{
		this->render_targets.clear();
		if(this->output == nullptr)
		{
			// we use window output. swapchain and device depth.
			auto& swapchain = tz::gl::get_device2().get_swapchain();
			const std::uint32_t frame_in_flight_count = swapchain.get_images().size();
			this->render_targets.resize(frame_in_flight_count);
			for(std::size_t i = 0; i < frame_in_flight_count; i++)
			{
				auto& colours = this->render_targets[i].colour_attachments;
				colours.push_back
				(vk2::ImageView{{
					.image = &swapchain.get_images()[i],
					.aspect = vk2::ImageAspectFlag::Colour
				 }});
				this->render_targets[i].depth_attachment = vk2::ImageView
				{{
					.image = &tz::gl::get_device2().get_depth_image(),
					.aspect = vk2::ImageAspectFlag::Depth
				}};
			}
		}
	}

//--------------------------------------------------------------------------------------------------

	renderer_command_processor::renderer_command_processor(const tz::gl::renderer_info& rinfo):
	renderer_output_manager(rinfo),
	render_wait_enabled(rinfo.get_options().contains(tz::gl::renderer_option::render_wait))
	{
		TZ_PROFZONE("renderer_command_processor - initialise", 0xFFAAAA00);
		this->allocate_commands(command_type::both);
		this->scratch_initialise_static_resources();
	}

	renderer_command_processor::~renderer_command_processor()
	{
		this->free_commands(command_type::both);
	}

	void renderer_command_processor::do_scratch_work(std::function<void(vk2::CommandBufferRecording&)> record_commands)
	{
		TZ_PROFZONE("renderer_command_processor - scratch work...", 0xFFAAAA00);
		if(this->scratch_command_buffer().command_count() > 0)
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
		tz::gl::get_device2().vk_submit_and_run_commands_blocking(renderer_vulkan_base::uid, scratch_id, 0, buf);
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
		}
		for(unsigned int i = 0; i < bufs.size(); i++)
		{
			vk2::CommandBufferRecording record = bufs[i].record();
			work_record_commands(record, i);
		}
	}

	constexpr std::size_t cmdbuf_work_alloc_id = 0;
	constexpr std::size_t cmdbuf_scratch_alloc_id = 1;

	void renderer_command_processor::allocate_commands(renderer_command_processor::command_type t)
	{
		TZ_PROFZONE("renderer_command_processor - allocate commands", 0xFFAAAA00);
		if(this->command_allocations.size() > 0 /* this might be incorrect logic */)
		{
			this->free_commands(t);
		}
		this->command_allocations.resize(2);
		const std::uint32_t frame_in_flight_count = tz::gl::get_device2().get_swapchain().get_images().size();
		const std::size_t scratch_buffer_count = 1;
		// we want a quantity of command buffers equal to `frame_in_flight_count` + 1.
		// the +1 is a scratch buffer which will be used for various commands (mainly renderer edits, but a few things in setup aswell, such as static resource writes).

		// tell the command pool about our intentions.
		auto& dev = tz::gl::get_device2();
		dev.vk_command_pool_touch(renderer_vulkan_base::uid,
		{
			// todo: no magic values.
			.compute = false,
			.requires_present = true
		});
		// one allocation for our render/compute work commands.
		this->command_allocations[cmdbuf_work_alloc_id] = dev.vk_allocate_commands
		({
			.buffer_count = frame_in_flight_count
		}, renderer_vulkan_base::uid);
		this->command_allocations[cmdbuf_scratch_alloc_id] = dev.vk_allocate_commands
		({
			.buffer_count = scratch_buffer_count
		}, renderer_vulkan_base::uid);
	}

	void renderer_command_processor::free_commands(renderer_command_processor::command_type t)
	{
		TZ_PROFZONE("renderer_command_processor - free commands", 0xFFAAAA00);
		if(this->command_allocations.empty())
		{
			return;
		}
		if(t == command_type::work || t == command_type::both)
		{
			tz::gl::get_device2().vk_free_commands(renderer_vulkan_base::uid, cmdbuf_work_alloc_id, this->command_allocations[cmdbuf_work_alloc_id].buffers);
			// pop_front() doesnt exist in std::vector ugh
			this->command_allocations.erase(this->command_allocations.begin());
		}
		if(t == command_type::scratch || t == command_type::both)
		{
			tz::gl::get_device2().vk_free_commands(renderer_vulkan_base::uid, cmdbuf_scratch_alloc_id, this->command_allocations[cmdbuf_scratch_alloc_id].buffers);
			this->command_allocations.pop_back();
		}
		if(t == command_type::both)
		{
			tz::assert(this->command_allocations.empty());
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
						.device = &tz::gl::get_device2().vk_get_logical_device(),
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
						.device = &tz::gl::get_device2().vk_get_logical_device(),
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
							.source_stage = vk2::PipelineStage::Top,
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
	state(rinfo.state())
	{
		TZ_PROFZONE("renderer_vulkan2 - initialise", 0xFFAAAA00);

	}

	const ioutput* renderer_vulkan2::get_output() const
	{
		tz::error("NYI");
		return nullptr;
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
		tz::error("NYI");
	}

	void renderer_vulkan2::edit(tz::gl::renderer_edit_request req)
	{
		(void)req;
		tz::error("NYI");
	}

	void renderer_vulkan2::dbgui()
	{
		tz::error("NYI");
	}

	std::string_view renderer_vulkan2::debug_get_name() const
	{
		return "NYI";
	}

	renderer_vulkan2 renderer_vulkan2::null()
	{
		return {};
	}

	bool renderer_vulkan2::is_null() const
	{
		tz::error("NYI");
		return true;
	}
}
