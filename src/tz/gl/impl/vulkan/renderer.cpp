#if TZ_VULKAN
#include "tz/core/profile.hpp"
#include "tz/core/debug.hpp"
#include "tz/dbgui/dbgui.hpp"
#include "tz/gl/declare/image_format.hpp"
#include "tz/gl/impl/vulkan/device.hpp"
#include "tz/gl/impl/vulkan/renderer.hpp"
#include "tz/gl/impl/vulkan/component.hpp"
#include "tz/gl/impl/vulkan/convert.hpp"
#include "tz/gl/impl/vulkan/detail/sampler.hpp"
#include "tz/gl/impl/vulkan/detail/tz_vulkan.hpp"
#include "tz/gl/impl/vulkan/detail/fixed_function.hpp"
#include "tz/gl/impl/vulkan/detail/gpu_mem.hpp"
#include "tz/gl/impl/vulkan/detail/descriptors.hpp"
#include "tz/gl/impl/vulkan/detail/image_view.hpp"
#include "tz/gl/output.hpp"

namespace tz::gl
{
	using namespace tz::gl;

//--------------------------------------------------------------------------------------------------
	// Utility free-functions.
	vk2::SamplerInfo make_fitting_sampler(const iresource& res)
	{
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

//--------------------------------------------------------------------------------------------------
	ResourceStorage::ResourceStorage(const renderer_info& info):
	AssetStorageCommon<iresource>(info.get_resources()),
	frame_in_flight_count(get_device().get_device_window().get_output_images().size())
	{
		TZ_PROFZONE("Vulkan Frontend - renderer_vulkan ResourceStorage Create", 0xFFAAAA00);
		const vk2::LogicalDevice& ldev = get_device().vk_get_logical_device();
		this->samplers.reserve(this->count());

		auto resources = info.get_resources();
		std::size_t encountered_reference_count = 0;

		auto retrieve_resource_metadata = [this](icomponent* cmp)
		{
			iresource* res = cmp->get_resource();
			switch(res->get_type())
			{
				case resource_type::buffer:
				{
					// If the buffer is dynamic, let's link up the resource data span now.
					if(res->get_access() == resource_access::dynamic_fixed || res->get_access() == resource_access::dynamic_variable)
					{
						std::span<const std::byte> initial_data = res->data();
						std::span<std::byte> buffer_byte_data = this->components.back().as<buffer_component_vulkan>()->vk_get_buffer().map_as<std::byte>();
						std::copy(initial_data.begin(), initial_data.end(), buffer_byte_data.begin());
						res->set_mapped_data(buffer_byte_data);
					}
				}
				break;
				case resource_type::image:
				{
					this->samplers.emplace_back(make_fitting_sampler(*res));

					auto* img = static_cast<image_component_vulkan*>(cmp);
					// We will need to create an image view. Let's get that out-of-the-way-now.
					vk2::Image& underlying_image = img->vk_get_image();
					this->image_component_views.emplace_back
						(vk2::ImageViewInfo{
							.image = &underlying_image,
							.aspect = vk2::derive_aspect_from_format(underlying_image.get_format()).front()
						 });
					// If the image is dynamic, let's link up the resource data span now.
					if(res->get_access() == resource_access::dynamic_fixed || res->get_access() == resource_access::dynamic_variable)
					{

						std::span<const std::byte> initial_data = res->data();
						std::span<std::byte> image_data = {reinterpret_cast<std::byte*>(underlying_image.map()), initial_data.size_bytes()};
						std::copy(initial_data.begin(), initial_data.end(), image_data.begin());
						res->set_mapped_data(image_data);
					}
				}
				break;
				default:
					tz::error("Unrecognised resource_type. Please submit a bug report.");
				break;
			}
		};

		this->components.reserve(this->count());
		for(std::size_t i = 0; i < this->count(); i++)
		{
			iresource* res = this->get(static_cast<tz::hanval>(i));
			icomponent* comp = nullptr;
			if(res == nullptr)
			{
				// If we see a null resource, it means we're looking for a component (resource reference).
				comp = const_cast<icomponent*>(info.get_components()[encountered_reference_count]);
				this->components.push_back(comp);
				encountered_reference_count++;

				res = comp->get_resource();
				// Also we'll write into the asset storage so it doesn't still think the resource is null.
				this->set(static_cast<tz::hanval>(i), res);
			}
			else
			{
				switch(res->get_type())
				{
					case resource_type::buffer:
					{
						this->components.push_back(tz::make_owned<buffer_component_vulkan>(*res));
					}
					break;
					case resource_type::image:
						this->components.push_back(tz::make_owned<image_component_vulkan>(*res));
					break;
					default:
						tz::error("Unrecognised resource_type. Please submit a bug report.");
					break;
				}
				comp = this->components.back().get();
			}
			retrieve_resource_metadata(comp);
		}

		std::size_t descriptor_buffer_count = this->resource_count_of(resource_type::buffer);
		if(info.state().graphics.index_buffer != tz::nullhand)
		{
			descriptor_buffer_count--;
		}
		if(info.state().graphics.draw_buffer != tz::nullhand)
		{
			descriptor_buffer_count--;
		}
		{
			vk2::DescriptorLayoutBuilder lbuilder;
			lbuilder.set_device(ldev);
			// Each buffer gets their own binding id.
			for(std::size_t i = 0; i < descriptor_buffer_count; i++)
			{
				// TODO: Only add the necessary flags to the buffers with variable access instead of all of them if any have it. Dependent on changes to sync_descriptors.
				vk2::DescriptorFlags desc_flags = {vk2::DescriptorFlag::UpdateAfterBind, vk2::DescriptorFlag::UpdateUnusedWhilePending};

				lbuilder.with_binding
				({
					.type = vk2::DescriptorType::StorageBuffer,
					.count = 1,
					.flags = desc_flags
				});
			}
			// And one giant descriptor array for all textures. If there aren't any image resources though, we won't bother.
			if(this->resource_count_of(resource_type::image))
			{
				lbuilder.with_binding
				({
					.type = vk2::DescriptorType::ImageWithSampler,
					.count = static_cast<std::uint32_t>(this->image_component_views.size()),
					.flags = {	vk2::DescriptorFlag::PartiallyBound,
							vk2::DescriptorFlag::UpdateAfterBind,
							vk2::DescriptorFlag::UpdateUnusedWhilePending
						}
				});
			}
			this->descriptor_layout = lbuilder.build();
		}

		// If we have no shader resources at all, then we completely skip creating pools and sets.
		if(this->descriptor_empty())
		{
			return;
		}
		// Create pool limits. Enough for all of our resources. However, if we don't have any of a specific resource, we shouldn't add a limit at all for it (zero-size limits are not allowed).
		decltype(std::declval<vk2::DescriptorPoolInfo::PoolLimits>().limits) limits;
		if(descriptor_buffer_count > 0)
		{
			limits[vk2::DescriptorType::StorageBuffer] = descriptor_buffer_count * this->frame_in_flight_count;
		}
		if(!this->image_component_views.empty())
		{
			limits[vk2::DescriptorType::ImageWithSampler] = this->image_component_views.size() * this->frame_in_flight_count;
		}

		this->descriptor_pool = 
		{vk2::DescriptorPoolInfo{
			.limits =
			{
				.limits = limits,
				.max_sets = static_cast<std::uint32_t>(this->frame_in_flight_count),
				.supports_update_after_bind = true
			},
			.logical_device = &ldev
		}};
		{
			tz::basic_list<const vk2::DescriptorLayout*> alloc_layout_list;
			for(std::size_t i = 0; i < this->frame_in_flight_count; i++)
			{
				alloc_layout_list.add(&this->descriptor_layout);
			}
			this->descriptors = this->descriptor_pool.allocate_sets
			({
				.set_layouts = std::move(alloc_layout_list)
			});
		};
		tz::assert(this->descriptors.success(), "Descriptor Pool allocation failed. Please submit a bug report.");
		this->sync_descriptors(true, info.state());
	}

	ResourceStorage::ResourceStorage(ResourceStorage&& move):
	AssetStorageCommon<iresource>(static_cast<AssetStorageCommon<iresource>&&>(move)),
	components(std::move(move.components)),
	image_component_views(std::move(move.image_component_views)),
	samplers(std::move(move.samplers)),
	descriptor_layout(std::move(move.descriptor_layout)),
	descriptor_pool(std::move(move.descriptor_pool)),
	descriptors(std::move(move.descriptors)),
	frame_in_flight_count(move.frame_in_flight_count)
	{
		for(auto& set : this->descriptors.sets)
		{
			set.set_layout(this->descriptor_layout);
		}
	}

	ResourceStorage& ResourceStorage::operator=(ResourceStorage&& rhs)
	{
		AssetStorageCommon<iresource>::operator=(std::move(rhs));
		std::swap(this->components, rhs.components);
		std::swap(this->image_component_views, rhs.image_component_views);
		std::swap(this->samplers, rhs.samplers);
		std::swap(this->descriptor_layout, rhs.descriptor_layout);
		std::swap(this->descriptor_pool, rhs.descriptor_pool);
		std::swap(this->descriptors, rhs.descriptors);
		std::swap(this->frame_in_flight_count, rhs.frame_in_flight_count);
		for(auto& set : this->descriptors.sets)
		{
			set.set_layout(this->descriptor_layout);
		}
		return *this;
	}
			

	const icomponent* ResourceStorage::get_component(resource_handle handle) const
	{
		if(handle == tz::nullhand)
		{
			return nullptr;
		}
		return this->components[static_cast<std::size_t>(static_cast<tz::hanval>(handle))].get();
	}

	icomponent* ResourceStorage::get_component(resource_handle handle)
	{
		if(handle == tz::nullhand)
		{
			return nullptr;
		}
		return this->components[static_cast<std::size_t>(static_cast<tz::hanval>(handle))].get();
	}

	const vk2::DescriptorLayout& ResourceStorage::get_descriptor_layout() const
	{
		return this->descriptor_layout;
	}

	std::span<const vk2::DescriptorSet> ResourceStorage::get_descriptor_sets() const
	{
		return this->descriptors.sets;
	}

	std::size_t ResourceStorage::resource_count_of(resource_type type) const
	{
		return std::count_if(this->components.begin(), this->components.end(),
		[type](const auto& component_ptr)
		{
			return component_ptr->get_resource()->get_type() == type;
		});
	}

	void ResourceStorage::notify_image_recreated(resource_handle image_resource_handle)
	{
		// image_component's underlying vk2::Image was recently replaced with another. This means this->image_component_views[id corresponding to handle] is wrong and needs to be remade.
		std::size_t img_view_idx = 0;
		auto handle_val = static_cast<std::size_t>(static_cast<tz::hanval>(image_resource_handle));
		for(std::size_t i = 0; i < handle_val; i++)
		{
			if(this->get(static_cast<tz::hanval>(i))->get_type() == resource_type::image)
			{
				img_view_idx++;
			}
		}
		auto& img = static_cast<image_component_vulkan*>(this->get_component(image_resource_handle))->vk_get_image();
		this->image_component_views[img_view_idx] =
		{{
			.image = &img,
			.aspect = vk2::derive_aspect_from_format(img.get_format()).front()
		}};
	}

	void ResourceStorage::sync_descriptors(bool write_everything, const render_state& state)
	{
		TZ_PROFZONE("Vulkan Frontend - renderer_vulkan ResourceStorage Descriptor Sync", 0xFFAAAA00);
		std::vector<buffer_component_vulkan*> buffers;
		buffers.reserve(this->components.size());
		for(auto& component_ptr : this->components)
		{
			if(component_ptr->get_resource()->get_type() == resource_type::buffer)
			{
				buffers.push_back(component_ptr.as<buffer_component_vulkan>());
			}
		}
		std::size_t descriptor_buffer_count = buffers.size();
		if(state.graphics.index_buffer != tz::nullhand)
		{
			descriptor_buffer_count--;
		}
		if(state.graphics.draw_buffer != tz::nullhand)
		{
			descriptor_buffer_count--;
		}

		// Now write the initial resources into their descriptors.
		vk2::DescriptorPool::UpdateRequest update = this->descriptor_pool.make_update_request();
		// For each set, make the same edits.
		for(std::size_t i = 0 ; i < this->frame_in_flight_count; i++)
		{
			vk2::DescriptorSet& set = this->descriptors.sets[i];
			vk2::DescriptorSet::EditRequest set_edit = set.make_edit_request();
			// Now update each binding corresponding to a buffer resource.
			for(std::size_t j = 0; j < buffers.size(); j++)
			{
				buffer_component_vulkan& comp = *buffers[j];
				if(comp.get_resource() == this->get(state.graphics.index_buffer) || comp.get_resource() == this->get(state.graphics.draw_buffer))
				{
					continue;
				}
				set_edit.set_buffer(j,
				{
					.buffer = &comp.vk_get_buffer(),
					.buffer_offset = 0,
					.buffer_write_size = comp.vk_get_buffer().size()
				});
			}
			// And finally the binding corresponding to the texture resource descriptor array
			// Note, we only do this if we're writing to everything (we should only ever do this once because the image resources are not update-after-bind-bit!
			if(write_everything)
			{
				for(std::size_t j = 0; j < this->image_component_views.size(); j++)
				{
					set_edit.set_image(descriptor_buffer_count,
					{
						.sampler = &this->samplers[j],
						.image_view = &this->image_component_views[j]
					}, j);
				}
			}
			update.add_set_edit(set_edit);
		}
		this->descriptor_pool.update_sets(update);
	}

	bool ResourceStorage::empty() const
	{
		return this->count() == 0;
	}
	
	bool ResourceStorage::descriptor_empty() const
	{
		return this->descriptor_layout.binding_count() == 0;
	}

	void ResourceStorage::write_padded_image_data()
	{
		for(auto& component_ptr : this->components)
		{
			iresource* res = component_ptr->get_resource();
			if(res->get_type() == resource_type::image && res->get_access() != resource_access::static_fixed)
			{
				const vk2::Image& img = component_ptr.as<const image_component_vulkan>()->vk_get_image();
				// If it's dynamic in any way, the user might have written changes.
				// The user writes image data assuming the rows are tightly-packed, but this is not at all guaranteed (infact its highly unlikely). We will correct the written data each time.
				std::span<std::byte> d = res->data();
				// How many rows do we have?
				std::size_t num_rows = img.get_dimensions()[1];
				std::size_t row_pitch = img.get_linear_row_length();
				// Find out how many bytes the resource data takes up per row.
				std::size_t resource_row_pitch = d.size_bytes() / num_rows;
				if(row_pitch == resource_row_pitch)
				{
					// Image row data really is tightly packed.
					continue;
				}
				tz::assert(row_pitch > resource_row_pitch, "Linear CPU vk2::Image row data is negatively-padded? Assuming image_resource data is tightly-packed, it uses %zu bytes per row, but the actual image uses %zu? It should be using more, not less!", resource_row_pitch, row_pitch);
				/*
					// Consider the image resource data for a 3x3 image, there are 2 extra padding bytes which we want to fix.
					// The next row needs to start X padding bytes later.
				 	xxx**
					xxx**
					xxx**
					// Whereas what we have rn is:
					xxxxx
					xxxx*
					*****
				 */
				std::vector<char> buffer;
				buffer.resize(row_pitch * num_rows, '\0');
				for(std::size_t row_id = 0; row_id < num_rows; row_id++)
				{
					// Copy each row.
					// We need a separate temp buffer to work with. Remember that the resource data and the mapped image data are actually the same thing!
					std::memcpy(buffer.data() + (row_id * row_pitch), d.data() + (row_id * resource_row_pitch), row_pitch);
				}
				// Finally, copy the whole buffer into the actual image data.
				std::memcpy(d.data(), buffer.data(), row_pitch * num_rows);
			}
		}
	}

//--------------------------------------------------------------------------------------------------

	OutputManager::OutputManager(const renderer_info& info):
	output(info.get_output() != nullptr ? info.get_output()->unique_clone() : nullptr),
	ldev(&get_device().vk_get_logical_device()),
	swapchain_images(get_device().get_device_window().get_output_images()),
	swapchain_depth_images(&get_device().get_device_window().get_depth_image()),
	options(info.get_options())
	{
		TZ_PROFZONE("Vulkan Frontend - renderer_vulkan OutputManager Create", 0xFFAAAA00);
		this->create_output_resources(this->swapchain_images, this->swapchain_depth_images);
	}

	OutputManager::OutputManager(OutputManager&& move):
	output(std::move(move.output)),
	ldev(move.ldev),
	swapchain_images(std::move(move.swapchain_images)),
	swapchain_depth_images(std::move(move.swapchain_depth_images)),
	options(std::move(move.options)),
	output_imageviews(std::move(move.output_imageviews)),
	output_depth_imageviews(std::move(move.output_depth_imageviews)),
	render_pass(std::move(move.render_pass)),
	output_framebuffers(std::move(move.output_framebuffers))
	{
		for(auto& fb : this->output_framebuffers)
		{
			fb.set_render_pass(this->render_pass);
		}
	}

	OutputManager& OutputManager::operator=(OutputManager&& rhs)
	{
		std::swap(this->output, rhs.output);
		std::swap(this->ldev, rhs.ldev);
		std::swap(this->swapchain_images, rhs.swapchain_images);
		std::swap(this->swapchain_depth_images, rhs.swapchain_depth_images);
		std::swap(this->options, rhs.options);
		std::swap(this->output_imageviews, rhs.output_imageviews);
		std::swap(this->output_depth_imageviews, rhs.output_depth_imageviews);
		std::swap(this->render_pass, rhs.render_pass);
		std::swap(this->output_framebuffers, rhs.output_framebuffers);
		if(!this->render_pass.is_null())
		{
			for(auto& fb : this->output_framebuffers)
			{
				fb.set_render_pass(this->render_pass);
			}
		}
		return *this;
	}

	const vk2::RenderPass& OutputManager::get_render_pass() const
	{
		return this->render_pass;
	}

	std::vector<OutputImageState> OutputManager::get_output_images()
	{
		// This depends on whether we're rendering into the window or we have an image output.
		if(this->output == nullptr || this->output->get_target() == output_target::window)
		{
			// We're rendering directly into the window, so we just use the swapchain images.
			std::vector<OutputImageState> ret(this->swapchain_images.size());
			for(std::size_t i = 0; i < this->swapchain_images.size(); i++)
			{
				ret[i] =
				{
					.colour_attachments = {&this->swapchain_images[i]},
					.depth_attachment = this->swapchain_depth_images
				};
			}
			return ret;
		}
		else if(this->output->get_target() == output_target::offscreen_image)
		{
			// We have been provided an image_output which will contain an image_component_vulkan. We need to retrieve that image and return a span covering it.
			auto& out = static_cast<image_output&>(*this->output);

			OutputImageState out_image;
			for(std::size_t i = 0; i < out.colour_attachment_count(); i++)
			{
				out_image.colour_attachments.add(&out.get_colour_attachment(i).vk_get_image());
			}
			if(out.has_depth_attachment())
			{
				out_image.depth_attachment = &out.get_depth_attachment().vk_get_image();
			}
			else
			{
				// Now we weren't given a depth image. We should leave this nullptr, but only if no_depth_testing was enabled. If we need to do depth testing but weren't given a depth image, we'll use the swapchain depth image.
				if(this->options.contains(renderer_option::no_depth_testing))
				{
					out_image.depth_attachment = nullptr;
				}
				else
				{
					out_image.depth_attachment = this->swapchain_depth_images;
				}
			}

			std::vector<OutputImageState> ret(this->swapchain_images.size(), out_image);
			tz::assert(!out.has_depth_attachment(), "Depth attachment on an image_output is not yet implemented");
			return ret;
		}
		else
		{
			tz::error("Unrecognised output_target. Please submit a bug report.");
			return {};
		}
	}

	std::span<const vk2::Framebuffer> OutputManager::get_output_framebuffers() const
	{
		return this->output_framebuffers;
	}

	std::span<vk2::Framebuffer> OutputManager::get_output_framebuffers()
	{
		return this->output_framebuffers;
	}

	tz::vec2ui OutputManager::get_output_dimensions() const
	{
		tz::assert(!this->output_imageviews.empty(), "OutputManager had no output views, so impossible to retrieve viewport dimensions. Please submit a bug report.");
		return this->output_imageviews.front().colour_views.front().get_image().get_dimensions();
	}

	ioutput* OutputManager::get_output()
	{
		return this->output.get();
	}

	const ioutput* OutputManager::get_output() const
	{
		return this->output.get();
	}
	
	bool OutputManager::has_depth_images() const
	{
		return !this->options.contains(renderer_option::no_depth_testing);
	}

	void OutputManager::create_output_resources(std::span<vk2::Image> swapchain_images, vk2::Image* depth_image)
	{
		TZ_PROFZONE("Vulkan Frontend - renderer_vulkan OutputManager (Output Resources Creation)", 0xFFAAAA00);
		this->swapchain_images = swapchain_images;
		this->output_imageviews.clear();
		this->output_depth_imageviews.clear();
		this->output_framebuffers.clear();

		this->swapchain_depth_images = depth_image;
		this->output_depth_imageviews.reserve(this->swapchain_images.size());

		this->populate_output_views();

		#if TZ_DEBUG
			for(const OutputImageViewState& out_view : this->output_imageviews)
			{
				tz::assert(std::equal(out_view.colour_views.begin(), out_view.colour_views.end(), out_view.colour_views.begin(), [](const vk2::ImageView& a, const vk2::ImageView& b){return a.get_image().get_format() == b.get_image().get_format();}), "Detected that not every output image in a renderer_vulkan has the same format. This is not permitted as RenderPasses would not be compatible. Please submit a bug report.");
			}
		#endif // TZ_DEBUG

		this->make_render_pass();
		this->populate_framebuffers();
	}


	void OutputManager::populate_output_views()
	{
		auto output_image_copy = this->get_output_images();
		for(std::size_t i = 0; i < output_image_copy.size(); i++)
		{
			// We need to pass image views around in various places within the vulkan api. We create them here.
			const OutputImageState& out_image = output_image_copy[i];
			OutputImageViewState out_view;
			for(vk2::Image* colour_img : out_image.colour_attachments)
			{
				out_view.colour_views.add({vk2::ImageViewInfo{
					.image = colour_img,
					.aspect = vk2::ImageAspectFlag::Colour
				}});
			}
			this->output_imageviews.push_back(std::move(out_view));

			if(out_image.depth_attachment != nullptr)
			{
				this->output_depth_imageviews.push_back
				(vk2::ImageViewInfo{
					.image = out_image.depth_attachment,
					.aspect = vk2::ImageAspectFlag::Depth
				});
			}
		}
	}

	void OutputManager::make_render_pass()
	{
		// Now create an ultra-basic renderpass.
		// We're matching the image_format of the provided output image.
		vk2::ImageLayout final_layout;
		if(this->output == nullptr || this->output->get_target() == output_target::window)
		{
			final_layout = vk2::ImageLayout::Present;
		}
		else if(this->output->get_target() == output_target::offscreen_image)
		{
			final_layout = vk2::ImageLayout::ShaderResource;
		}
		else
		{
			final_layout = vk2::ImageLayout::Undefined;
			tz::error("Unknown RendererOutputType. Please submit a bug report.");
		}
		
		auto output_image_copy = this->get_output_images();
		// Our renderpass is no longer so simple with the possibilty of multiple colour outputs.
		std::uint32_t colour_output_length = output_image_copy.front().colour_attachments.length();
		const bool use_depth_output = output_image_copy.front().depth_attachment != nullptr;

		vk2::RenderPassBuilder rbuilder;
		rbuilder.set_device(*this->ldev);
		for(vk2::Image* colour_image : output_image_copy.front().colour_attachments)
		{
			rbuilder.with_attachment
			({
				.format = colour_image->get_format(),
				.colour_depth_load = this->options.contains(renderer_option::no_clear_output) ? vk2::LoadOp::Load : vk2::LoadOp::Clear,
				.colour_depth_store = vk2::StoreOp::Store,
				.initial_layout = this->options.contains(renderer_option::no_clear_output) ? vk2::ImageLayout::Present : vk2::ImageLayout::Undefined,
				.final_layout = final_layout
			});
		}

		if(use_depth_output)
		{
			rbuilder.with_attachment
			({
				.format = output_image_copy.front().depth_attachment->get_format(),
				.colour_depth_load = this->options.contains(renderer_option::no_clear_output) ? vk2::LoadOp::Load : vk2::LoadOp::Clear,
				.colour_depth_store = this->options.contains(renderer_option::no_present) ? vk2::StoreOp::Store : vk2::StoreOp::DontCare,
				.initial_layout = this->options.contains(renderer_option::no_clear_output) ? vk2::ImageLayout::DepthStencilAttachment : vk2::ImageLayout::Undefined,
				.final_layout = vk2::ImageLayout::DepthStencilAttachment
			});
		}

		vk2::SubpassBuilder sbuilder;
		sbuilder.set_pipeline_context(vk2::PipelineContext::graphics);
		for(std::uint32_t i = 0; i < colour_output_length; i++)
		{
			sbuilder.with_colour_attachment
			({
				.attachment_idx = i,
				.current_layout = vk2::ImageLayout::ColourAttachment
			});
		}
		if(use_depth_output)
		{
			sbuilder.with_depth_stencil_attachment
			({
				.attachment_idx = colour_output_length,
				.current_layout = vk2::ImageLayout::DepthStencilAttachment
			});
		}

		this->render_pass = rbuilder.with_subpass(sbuilder.build()).build();

	}

	void OutputManager::populate_framebuffers()
	{
		const bool has_depth = !this->output_depth_imageviews.empty();
		if(has_depth)
		{
			tz::assert(this->output_imageviews.size() == this->output_depth_imageviews.size(), "Detected at least 1 output depth image views (%zu), but that doesn't match the number of output colour imageviews (%zu). Please submit a bug report", this->output_depth_imageviews.size(), this->output_imageviews.size());
		}
		for(std::size_t i = 0; i < this->output_imageviews.size(); i++)
		{
			tz::vec2ui dims = this->output_imageviews[i].colour_views.front().get_image().get_dimensions();
			tz::basic_list<vk2::ImageView*> attachments;
			attachments.resize(this->output_imageviews[i].colour_views.length());
			std::transform(this->output_imageviews[i].colour_views.begin(), this->output_imageviews[i].colour_views.end(), attachments.begin(),
			[](vk2::ImageView& colour_view)
			{
				return &colour_view;
			});
			if(has_depth)
			{
				attachments.add(&this->output_depth_imageviews[i]);
			}
			this->output_framebuffers.push_back
			(vk2::FramebufferInfo{
				.render_pass = &this->render_pass,
				.attachments = attachments,
				.dimensions = dims
			});
		}
	}

//--------------------------------------------------------------------------------------------------

	GraphicsPipelineManager::GraphicsPipelineManager(const renderer_info& info, const ResourceStorage& resources, const OutputManager& output)
	{
		this->shader = this->make_shader(get_device().vk_get_logical_device(), info.shader());
		this->pipeline_layout = this->make_pipeline_layout(resources.get_descriptor_layout(), get_device().get_device_window().get_output_images().size());
		this->depth_testing_enabled = !info.get_options().contains(renderer_option::no_depth_testing);
		const bool alpha_blending_enabled = info.get_options().contains(renderer_option::alpha_blending);
		this->graphics_pipeline = this->make_pipeline(output.get_output_dimensions(), depth_testing_enabled, alpha_blending_enabled, output.get_render_pass());
	}

	GraphicsPipelineManager::GraphicsPipelineManager(GraphicsPipelineManager&& move):
	shader(std::move(move.shader)),
	pipeline_layout(std::move(move.pipeline_layout)),
	graphics_pipeline(std::move(move.graphics_pipeline)),
	depth_testing_enabled(move.depth_testing_enabled)
	{
		if(!this->pipeline_layout.is_null())
		{
			this->graphics_pipeline.set_layout(this->pipeline_layout);
		}
	}

	GraphicsPipelineManager& GraphicsPipelineManager::operator=(GraphicsPipelineManager&& rhs)
	{
		std::swap(this->shader, rhs.shader);
		std::swap(this->pipeline_layout, rhs.pipeline_layout);
		std::swap(this->graphics_pipeline, rhs.graphics_pipeline);
		std::swap(this->depth_testing_enabled, rhs.depth_testing_enabled);

		if(!this->pipeline_layout.is_null())
		{
			this->graphics_pipeline.set_layout(this->pipeline_layout);
		}

		return *this;
	}

	const vk2::Pipeline& GraphicsPipelineManager::get_pipeline() const
	{
		return this->graphics_pipeline;
	}

	const vk2::Shader& GraphicsPipelineManager::get_shader() const
	{
		return this->shader;
	}

	vk2::Shader& GraphicsPipelineManager::get_shader()
	{
		return this->shader;
	}

	void GraphicsPipelineManager::recreate(const vk2::RenderPass& new_render_pass, tz::vec2ui new_viewport_dimensions, bool wireframe_mode)
	{
		this->wireframe_mode = wireframe_mode;
		TZ_PROFZONE("Vulkan Frontend - renderer_vulkan GraphicsPipelineManager Recreate", 0xFFAAAA00);
		if(this->is_compute())
		{
			this->graphics_pipeline =
			{vk2::ComputePipelineInfo{
				.shader = this->shader.native_data(),
				.pipeline_layout = &this->pipeline_layout,
				.device = &this->pipeline_layout.get_device()
			}};
		}
		else
		{
			this->graphics_pipeline =
			{{
				.shaders = this->shader.native_data(),
				.state = vk2::PipelineState
				{
					.viewport = vk2::create_basic_viewport(static_cast<tz::vec2>(new_viewport_dimensions)),
					.rasteriser = {.polygon_mode = (this->wireframe_mode ? vk2::PolygonMode::Line : vk2::PolygonMode::Fill)},
					.depth_stencil =
					{
						.depth_testing = this->depth_testing_enabled,
						.depth_writes = this->depth_testing_enabled
					},
					.dynamic =
					{
						.states = {vk2::DynamicStateType::Scissor}
					}
				},
				.pipeline_layout = &this->pipeline_layout,
				.render_pass = &new_render_pass,
				.device = &new_render_pass.get_device()
			 }};
		}
	}

	bool GraphicsPipelineManager::is_compute() const
	{
		return this->shader.is_compute();
	}

	bool GraphicsPipelineManager::is_wireframe_mode() const
	{
		return this->wireframe_mode;
	}

	vk2::Shader GraphicsPipelineManager::make_shader(const vk2::LogicalDevice& ldev, const shader_info& sinfo) const
	{
		TZ_PROFZONE("Vulkan Frontend - renderer_vulkan GraphicsPipelineManager (Shader Create)", 0xFFAAAA00);
		std::vector<char> vtx_src, frg_src, tesscon_src, tesseval_src, cmp_src;
		tz::basic_list<vk2::ShaderModuleInfo> modules;
		if(sinfo.has_shader(shader_stage::compute))
		{
			tz::assert(!sinfo.has_shader(shader_stage::vertex), "Shader has compute shader and vertex shader. These are mutually exclusive.");
			tz::assert(!sinfo.has_shader(shader_stage::fragment), "Shader has compute shader and fragment shader. These are mutually exclusive.");
			// Compute, we only care about the compute shader.
			{
				std::string_view compute_source = sinfo.get_shader(shader_stage::compute);
				cmp_src.resize(compute_source.length());
				std::copy(compute_source.begin(), compute_source.end(), cmp_src.begin());
			}
			modules =
			{
				{
					.device = &ldev,
					.type = vk2::ShaderType::compute,
					.code = cmp_src
				}
			};
		}
		else
		{
			// Graphics, must contain a Vertex and Fragment shader.
			tz::assert(sinfo.has_shader(shader_stage::vertex), "shader_info must contain a non-empty vertex shader if no compute shader is present.");
			tz::assert(sinfo.has_shader(shader_stage::fragment), "shader_info must contain a non-empty fragment shader if no compute shader is present.");
			{
				std::string_view vertex_source = sinfo.get_shader(shader_stage::vertex);
				vtx_src.resize(vertex_source.length());
				std::copy(vertex_source.begin(), vertex_source.end(), vtx_src.begin());

				std::string_view fragment_source = sinfo.get_shader(shader_stage::fragment);
				frg_src.resize(fragment_source.length());
				std::copy(fragment_source.begin(), fragment_source.end(), frg_src.begin());
			}
			modules = 
			{
				{
					.device = &ldev,
					.type = vk2::ShaderType::vertex,
					.code = vtx_src
				},
				{
					.device = &ldev,
					.type = vk2::ShaderType::fragment,
					.code = frg_src
				}
			};
			// Add optional shader stages.
			if(sinfo.has_shader(shader_stage::tessellation_control) || sinfo.has_shader(shader_stage::tessellation_evaluation))
			{
				tz::assert(sinfo.has_shader(shader_stage::tessellation_control) && sinfo.has_shader(shader_stage::tessellation_evaluation), "Detected a tessellaton shader type, but it was missing its sister. If a control or evaluation shader exists, they both must exist.");

				std::string_view tesscon_source = sinfo.get_shader(shader_stage::tessellation_control);
				tesscon_src.resize(tesscon_source.length());
				std::copy(tesscon_source.begin(), tesscon_source.end(), tesscon_src.begin());

				std::string_view tesseval_source = sinfo.get_shader(shader_stage::tessellation_evaluation);
				tesseval_src.resize(tesseval_source.length());
				std::copy(tesseval_source.begin(), tesseval_source.end(), tesseval_src.begin());
				modules.add(vk2::ShaderModuleInfo
				{
					.device = &ldev,
					.type = vk2::ShaderType::tessellation_control,
					.code = tesscon_src
				});
				modules.add(vk2::ShaderModuleInfo
				{
					.device = &ldev,
					.type = vk2::ShaderType::tessellation_evaluation,
					.code = tesseval_src
				});
			}
		}
		return
		{{
			.device = &ldev,
			.modules = modules
		}};
	}

	vk2::PipelineLayout GraphicsPipelineManager::make_pipeline_layout(const vk2::DescriptorLayout& dlayout, std::size_t frame_in_flight_count) const
	{
		std::vector<const vk2::DescriptorLayout*> layout_ptrs(frame_in_flight_count, &dlayout);
		return
		{{
			.descriptor_layouts = std::move(layout_ptrs),
			.logical_device = &dlayout.get_device()
		}};
	}

	vk2::GraphicsPipelineInfo GraphicsPipelineManager::make_graphics_pipeline(tz::vec2ui viewport_dimensions, bool depth_testing_enabled, bool alpha_blending_enabled, const vk2::RenderPass& render_pass) const
	{
		tz::basic_list<vk2::ColourBlendState::AttachmentState> alpha_blending_options;
		alpha_blending_options.resize(render_pass.get_info().total_colour_attachment_count());
		std::fill(alpha_blending_options.begin(), alpha_blending_options.end(), alpha_blending_enabled ? vk2::ColourBlendState::alpha_blending() : vk2::ColourBlendState::no_blending());
		return
		{
			.shaders = this->shader.native_data(),
			.state =
			{
				.viewport = vk2::create_basic_viewport(static_cast<tz::vec2>(viewport_dimensions)),
				.depth_stencil =
				{
					.depth_testing = depth_testing_enabled,
					.depth_writes = depth_testing_enabled
				},
				.colour_blend =
				{
					.attachment_states = alpha_blending_options,
					.logical_operator = VK_LOGIC_OP_COPY
				},
				.dynamic =
				{
					.states = {vk2::DynamicStateType::Scissor}
				}
			},
			.pipeline_layout = &this->pipeline_layout,
			.render_pass = &render_pass,
			.device = &this->pipeline_layout.get_device()
		};
	}

	vk2::ComputePipelineInfo GraphicsPipelineManager::make_compute_pipeline() const
	{
		return
		{
			.shader = this->shader.native_data(),
			.pipeline_layout = &this->pipeline_layout,
			.device = &this->pipeline_layout.get_device()
		};
	}

	vk2::Pipeline GraphicsPipelineManager::make_pipeline(tz::vec2ui viewport_dimensions, bool depth_testing_enabled, bool alpha_blending_enabled, const vk2::RenderPass& render_pass) const
	{
		if(this->is_compute())
		{
			return {this->make_compute_pipeline()};
		}

		return {this->make_graphics_pipeline(viewport_dimensions, depth_testing_enabled, alpha_blending_enabled, render_pass)};
	}

//--------------------------------------------------------------------------------------------------

	CommandProcessor::CommandProcessor(const renderer_info& info)
	{
		if(info.get_output() == nullptr || info.get_output()->get_target() == output_target::window)
		{
			this->requires_present = true;
		}
		this->instant_compute_enabled = info.get_options().contains(renderer_option::render_wait);
		this->graphics_queue = get_device().vk_get_logical_device().get_hardware_queue
		({
			.field = {vk2::QueueFamilyType::graphics},
			.present_support = this->requires_present
		});
		this->compute_queue = get_device().vk_get_logical_device().get_hardware_queue
		({
			.field = {vk2::QueueFamilyType::compute},
			.present_support = false
		});
		this->command_pool = vk2::CommandPool{{.queue = this->graphics_queue, .flags = {vk2::CommandPoolFlag::Reusable}}};
		this->frame_in_flight_count = get_device().get_device_window().get_output_images().size();
		this->commands = this->command_pool.allocate_buffers
		({
			.buffer_count = static_cast<std::uint32_t>(this->frame_in_flight_count + 1)
		});
		this->images_in_flight.resize(this->frame_in_flight_count, nullptr);
		this->options = info.get_options();
		this->device_scheduler = &get_device().get_render_scheduler();

		tz::assert(this->graphics_queue != nullptr, "Could not retrieve graphics present queue. Either your machine does not meet requirements, or (more likely) a logical error. Please submit a bug report.");
		tz::assert(this->compute_queue != nullptr, "Could not retrieve compute queue. Either your machine does not meet requirements, or (more likely) a logical error. Please submit a bug report.");
		tz::assert(this->commands.success(), "Failed to allocate from CommandPool");
	}

	CommandProcessor::CommandProcessor(CommandProcessor&& move):
	requires_present(move.requires_present),
	instant_compute_enabled(move.instant_compute_enabled),
	graphics_queue(move.graphics_queue),
	compute_queue(move.compute_queue),
	command_pool(std::move(move.command_pool)),
	commands(std::move(move.commands)),
	frame_in_flight_count(move.frame_in_flight_count),
	images_in_flight(std::move(move.images_in_flight)),
	options(move.options),
	device_scheduler(move.device_scheduler),
	output_image_index(move.output_image_index),
	current_frame(move.current_frame)
	{
		for(vk2::CommandBuffer& cbuf : this->commands.buffers)
		{
			cbuf.set_owner(this->command_pool);
		}
	}

	CommandProcessor& CommandProcessor::operator=(CommandProcessor&& rhs)
	{
		std::swap(this->requires_present, rhs.requires_present);
		std::swap(this->instant_compute_enabled, rhs.instant_compute_enabled);
		std::swap(this->graphics_queue, rhs.graphics_queue);
		std::swap(this->compute_queue, rhs.compute_queue);
		std::swap(this->command_pool,rhs.command_pool);
		std::swap(this->commands, rhs.commands);
		std::swap(this->frame_in_flight_count, rhs.frame_in_flight_count);
		std::swap(this->images_in_flight, rhs.images_in_flight);
		std::swap(this->options, rhs.options);
		std::swap(this->device_scheduler, rhs.device_scheduler);
		std::swap(this->output_image_index, rhs.output_image_index);
		std::swap(this->current_frame, rhs.current_frame);
		for(vk2::CommandBuffer& cbuf : this->commands.buffers)
		{
			cbuf.set_owner(this->command_pool);
		}
		for(vk2::CommandBuffer& cbuf : rhs.commands.buffers)
		{
			cbuf.set_owner(rhs.command_pool);
		}
		return *this;
	}

	std::span<const vk2::CommandBuffer> CommandProcessor::get_render_command_buffers() const
	{
		return {this->commands.buffers.begin(), this->frame_in_flight_count};
	}

	std::span<vk2::CommandBuffer> CommandProcessor::get_render_command_buffers()
	{
		return {this->commands.buffers.begin(), this->frame_in_flight_count};
	}

	CommandProcessor::RenderWorkSubmitResult CommandProcessor::do_render_work()
	{
		TZ_PROFZONE("renderer_vulkan - do render work", 0xFFAAAA00);
		auto& device_window = get_device().get_device_window();
		// Submit & Present
		{
			TZ_PROFZONE("wait on frame fence", 0xFFAAAA00);
			this->device_scheduler->get_frame_fences()[this->current_frame].wait_until_signalled();
		}
		bool already_have_image = device_window.has_unused_image();
		if(requires_present)
		{
			vk2::Semaphore* signal_sem = nullptr;
			if(!already_have_image)
			{
				signal_sem = &this->device_scheduler->get_image_signals()[current_frame];
			}
			this->output_image_index = device_window.get_unused_image
			({
				.signal_semaphore = signal_sem
			}).image_index;

			const vk2::Fence*& target_image = this->images_in_flight[this->output_image_index];
			if(target_image != nullptr)
			{
				TZ_PROFZONE("wait for swapchain image", 0xFFAAAA00);
				target_image->wait_until_signalled();
			}
			target_image = &this->device_scheduler->get_frame_fences()[this->output_image_index];
		}
		else
		{
			this->output_image_index = this->current_frame;
		}

		this->device_scheduler->get_frame_fences()[this->current_frame].unsignal();
		tz::basic_list<vk2::hardware::Queue::SubmitInfo::WaitInfo> waits;
		if(requires_present && !already_have_image)
		{
			waits =
			{
				vk2::hardware::Queue::SubmitInfo::WaitInfo
				{
					.wait_semaphore = &this->device_scheduler->get_image_signals()[this->current_frame],
					.wait_stage = vk2::PipelineStage::ColourAttachmentOutput
				}
			};
		}
		tz::basic_list<vk2::hardware::Queue::SubmitInfo::SignalInfo> signals;
		if(requires_present && !this->options.contains(renderer_option::no_present))
		{
			signals =
			{
				vk2::hardware::Queue::SubmitInfo::SignalInfo
				{
					.signal_semaphore = &this->device_scheduler->get_render_work_signals()[this->current_frame],
					.timeline = 0
				}
			};
		}
		this->graphics_queue->submit
		({
			.command_buffers = {&this->get_render_command_buffers()[this->output_image_index]},
			.waits = waits,
			.signals = signals,
			.execution_complete_fence = &this->device_scheduler->get_frame_fences()[this->current_frame]
		});

		if(this->instant_compute_enabled)
		{
			TZ_PROFZONE("block for render_wait", 0xFFAAAA00);
			this->device_scheduler->get_frame_fences()[this->current_frame].wait_until_signalled();
		}

		CommandProcessor::RenderWorkSubmitResult result;

		if(requires_present && !this->options.contains(renderer_option::no_present))
		{
			result.present = this->graphics_queue->present
			({
				.wait_semaphores = {&this->device_scheduler->get_render_work_signals()[this->current_frame]},
				.swapchain = &device_window.get_swapchain(),
				.swapchain_image_index = this->output_image_index
			});
			device_window.mark_image_used();
		}
		else
		{
			result.present = vk2::hardware::Queue::PresentResult::Success_NoIssue;
		}

		this->current_frame = (this->current_frame + 1) % this->frame_in_flight_count;
		return result;
	}

	void CommandProcessor::do_compute_work()
	{
		TZ_PROFZONE("Vulkan Frontend - renderer_vulkan CommandProcessor (Do Compute Work)", 0xFFAAAA00);

		this->device_scheduler->get_frame_fences()[this->current_frame].wait_until_signalled();
		this->device_scheduler->get_frame_fences()[this->current_frame].unsignal();
		this->compute_queue->submit
		({
			.command_buffers = {&this->get_render_command_buffers().front()},
			.waits = {},
			.signals = {},
			.execution_complete_fence = &this->device_scheduler->get_frame_fences()[this->current_frame]
		});

		if(this->instant_compute_enabled)
		{
			this->device_scheduler->get_frame_fences()[this->current_frame].wait_until_signalled();
		}
	}

	void CommandProcessor::wait_pending_commands_complete()
	{
		TZ_PROFZONE("Vulkan Frontend - renderer_vulkan CommandProcessor (Waiting on commands to complete)", 0xFFAAAA00);
		this->device_scheduler->wait_frame_work_complete();
	}

//--------------------------------------------------------------------------------------------------

	renderer_vulkan::renderer_vulkan(const renderer_info& info):
	ldev(&get_device().vk_get_logical_device()),
	options(info.get_options()),
	state(info.state()),
	resources(info),
	output(info),
	pipeline(info, resources, output),
	command(info),
	debug_name(info.debug_get_name())
	{
		if(this->options.contains(tz::gl::renderer_option::draw_indirect_count))
		{
			tz::assert(this->state.graphics.draw_buffer != tz::nullhand);
		}
		TZ_PROFZONE("Vulkan Frontend - renderer_vulkan Create", 0xFFAAAA00);
		this->window_dims_cache = tz::window().get_dimensions();

		this->setup_static_resources();
		this->setup_work_commands();

		// Debug name settings.
		#if TZ_DEBUG
			for(std::size_t i = 0; i < this->resource_count(); i++)
			{
				icomponent* comp = this->resources.get_component(static_cast<tz::hanval>(i));
				if(comp->get_resource()->get_type() == resource_type::buffer)
				{
					vk2::Buffer& buf = static_cast<buffer_component_vulkan*>(comp)->vk_get_buffer();
					std::string n = buf.debug_get_name();
					buf.debug_set_name(n + (n.empty() ? "" : " -> ") + this->debug_name + ":B" + std::to_string(i));
				}
				if(comp->get_resource()->get_type() == resource_type::image)
				{
					vk2::Image& img = static_cast<image_component_vulkan*>(comp)->vk_get_image();
					std::string n = img.debug_get_name();
					img.debug_set_name(n + (n.empty() ? "" : " -> ") + this->debug_name + ":I" + std::to_string(i));
				}
			}

			this->pipeline.get_shader().debug_set_name(this->debug_name);
		#endif
	}

	renderer_vulkan::renderer_vulkan(renderer_vulkan&& move):
	ldev(move.ldev),
	options(move.options),
	resources(std::move(move.resources)),
	output(std::move(move.output)),
	pipeline(std::move(move.pipeline)),
	command(std::move(move.command)),
	debug_name(std::move(move.debug_name)),
	scissor_cache(move.scissor_cache)
	{
	}

	renderer_vulkan::~renderer_vulkan()
	{
		if(this->is_null())
		{
			return;
		}
		this->ldev->wait_until_idle();
	}

	renderer_vulkan& renderer_vulkan::operator=(renderer_vulkan&& rhs)
	{
		std::swap(this->ldev, rhs.ldev);
		std::swap(this->options, rhs.options);
		std::swap(this->state, rhs.state);
		std::swap(this->resources, rhs.resources);
		std::swap(this->output, rhs.output);
		std::swap(this->pipeline, rhs.pipeline);
		std::swap(this->command, rhs.command);
		std::swap(this->debug_name, rhs.debug_name);
		std::swap(this->scissor_cache, rhs.scissor_cache);
		return *this;
	}

	unsigned int renderer_vulkan::resource_count() const
	{
		return this->resources.count();
	}

	const iresource* renderer_vulkan::get_resource(resource_handle handle) const
	{
		return this->resources.get(handle);
	}

	iresource* renderer_vulkan::get_resource(resource_handle handle)
	{
		return this->resources.get(handle);
	}

	const icomponent* renderer_vulkan::get_component(resource_handle handle) const
	{
		return this->resources.get_component(handle);
	}

	icomponent* renderer_vulkan::get_component(resource_handle handle)
	{
		return this->resources.get_component(handle);
	}

	ioutput* renderer_vulkan::get_output()
	{
		return this->output.get_output();
	}

	const ioutput* renderer_vulkan::get_output() const
	{
		return this->output.get_output();
	}

	const renderer_options& renderer_vulkan::get_options() const
	{
		return this->options;
	}

	const render_state& renderer_vulkan::get_state() const
	{
		return this->state;
	}

	void renderer_vulkan::render()
	{
		TZ_PROFZONE("Vulkan Frontend - renderer_vulkan Render", 0xFFAAAA00);
		if(tz::window().get_dimensions() == tz::vec2ui::zero())
		{
			TZ_PROFZONE("render() - wait till wsi event", 0xFFAAAA00);
			tz::wsi::wait_for_event();
			return;
		}
		if(tz::window().get_dimensions() != this->window_dims_cache)
		{
			TZ_PROFZONE("render() - handle resize", 0xFFAAAA00);
			this->handle_swapchain_outdated();
		}
		
		// If output scissor region has changed, we need to rerecord.
		if(this->get_output() != nullptr)
		{
			TZ_PROFZONE("render() - scissor rect change", 0xFFAA0000);
			if(this->get_output()->scissor != this->scissor_cache)
			{
				this->scissor_cache = this->get_output()->scissor;
				this->command.wait_pending_commands_complete();
				this->setup_work_commands();
			}
		}
		{
			TZ_PROFZONE("render() - dynamic image writes", 0xFFAA0000);
			this->resources.write_padded_image_data();
		}
		if(this->pipeline.is_compute())
		{
			this->command.do_compute_work();
		}
		else
		{
			CommandProcessor::RenderWorkSubmitResult result = this->command.do_render_work();
			switch(result.present)
			{
				case vk2::hardware::Queue::PresentResult::Success_Suboptimal:
				[[fallthrough]];
				case vk2::hardware::Queue::PresentResult::Success_NoIssue:
					this->present_failure_count = 0;
				break;
				case vk2::hardware::Queue::PresentResult::Fail_OutOfDate:
				{
					this->present_failure_count++;
					[[maybe_unused]] bool success = this->handle_swapchain_outdated();
					tz::assert(success, "Presentation failed - swapchain was out of date and recreation didn't help after %zu attempts. Please submit a bug report.", this->present_failure_count);
				}
				break;
				case vk2::hardware::Queue::PresentResult::Fail_AccessDenied:
					tz::error("Presentation failed - access denied. Please submit a bug report.");
				break;
				case vk2::hardware::Queue::PresentResult::Fail_SurfaceLost:
					tz::error("Presentation failed - surface lost. Please submit a bug report.");
				break;
				case vk2::hardware::Queue::PresentResult::Fail_FatalError:
					tz::error("Presentation failed - other fatal error. Please submit a bug report.");
				break;
				default:
					tz::error("Presentation failed, but for unknown reason. Please submit a bug report.");
				break;
			}
		}
	}

	void renderer_vulkan::edit(const renderer_edit_request& edit_request)
	{
		TZ_PROFZONE("Vulkan Frontend - renderer_vulkan Edit", 0xFFAAAA00);

		bool final_wireframe_mode_state = this->pipeline.is_wireframe_mode();
		if(edit_request.empty())
		{
			return;
		}
		renderer_vulkan::EditData data;
		this->command.wait_pending_commands_complete();
		for(const renderer_edit::variant& req : edit_request)
		{
			std::visit([this, &data, &final_wireframe_mode_state](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;
				if constexpr(std::is_same_v<T, renderer_edit::buffer_resize>)
				{
					  this->edit_buffer_resize(arg, data);
				}
				else if constexpr(std::is_same_v<T, renderer_edit::image_resize>)
				{
					this->edit_image_resize(arg, data);
				}
				else if constexpr(std::is_same_v<T, renderer_edit::resource_write>)
				{
					this->edit_resource_write(arg, data);
				}
				else if constexpr(std::is_same_v<T, renderer_edit::compute_config>)
				{
					this->edit_compute_config(arg, data);
				}
				else if constexpr(std::is_same_v<T, renderer_edit::render_config>)
				{
					if(arg.wireframe_mode != this->pipeline.is_wireframe_mode())
					{
						data.pipeline_recreate = true;
						final_wireframe_mode_state = arg.wireframe_mode;
					}
				}
				else if constexpr(std::is_same_v<T, renderer_edit::tri_count>)
				{
					if(arg.tri_count != this->state.graphics.tri_count)
					{
						this->state.graphics.tri_count = arg.tri_count;
						data.commands_rerecord = true;
					}
				}
				else if constexpr(std::is_same_v<T, renderer_edit::resource_reference>)
				{
					this->edit_resource_reference(arg, data);
				}
				else
				{
					tz::error("renderer Edit that is not yet supported has been requested. Please submit a bug report.");
				}
			}, req);
		}
		if(data == EditData{})
		{
			return;
		}
		if(data.descriptor_resync != 0)
		{
			this->resources.sync_descriptors(data.descriptor_resync & EditData::descriptor_resync_full, state);
		}
		if(data.pipeline_recreate)
		{
			this->pipeline.recreate(this->output.get_render_pass(), this->output.get_output_dimensions(), final_wireframe_mode_state);
			data.commands_rerecord = true;
		}
		if(data.commands_rerecord)
		{
			this->setup_work_commands();
		}
		if(data.static_resources_rewrite)
		{
			this->setup_static_resources();
		}
	}

	void renderer_vulkan::dbgui()
	{
		common_renderer_dbgui(*this);
	}

	std::string_view renderer_vulkan::debug_get_name() const
	{
		return this->debug_name;
	}

	renderer_vulkan renderer_vulkan::null()
	{
		return {};
	}
	
	bool renderer_vulkan::is_null() const
	{
		return this->ldev == nullptr;
	}

	void renderer_vulkan::edit_buffer_resize(renderer_edit::buffer_resize arg, EditData& data)
	{
		auto bufcomp = static_cast<buffer_component_vulkan*>(this->get_component(arg.buffer_handle));
		tz::assert(bufcomp != nullptr, "Invalid buffer handle in renderer_edit::buffer_resize");
		if(bufcomp->size() != arg.size)
		{
			bufcomp->resize(arg.size);

			if(bufcomp == this->get_component(this->state.graphics.index_buffer)
			|| bufcomp == this->get_component(this->state.graphics.draw_buffer))
			{
				// Index buffer has resized, meaning there will be a new underlying buffer object. This means the rendering commands need to be recorded because the bind command for the index buffer now references the dead old buffer.
				data.commands_rerecord = true;
			}
			data.descriptor_resync |= EditData::descriptor_resync_partial;
		}
	}

	void renderer_vulkan::edit_image_resize(renderer_edit::image_resize arg, EditData& data)
	{
		auto imgcomp = static_cast<image_component_vulkan*>(this->get_component(arg.image_handle));
		tz::assert(imgcomp != nullptr, "Invalid image handle in renderer_edit::image_resize");
		if(imgcomp->get_dimensions() != arg.dimensions)
		{
			imgcomp->resize(arg.dimensions);
			// An imageview was looking at the old image, let's update that.
			this->resources.notify_image_recreated(arg.image_handle);
			// New image so descriptor array needs to be re-written to.
			data.descriptor_resync |= EditData::descriptor_resync_full;
		}
	}
	
	void renderer_vulkan::edit_resource_write(renderer_edit::resource_write arg, [[maybe_unused]] EditData& data)
	{
		icomponent* comp = this->get_component(arg.resource);
		iresource* res = comp->get_resource();
		// Note: resource data won't change even though we change the buffer/image component. We need to set that aswell!
		switch(res->get_access())
		{
			case resource_access::static_fixed:
			{
				// Create staging buffer.
				vk2::Buffer staging_buffer
				{{
					.device = this->ldev,
					.size_bytes = arg.offset + arg.data.size_bytes(),
					.usage = {vk2::BufferUsage::TransferSource},
					.residency = vk2::MemoryResidency::CPU
				}};
				// Write data into staging buffer.
				{
					void* ptr = staging_buffer.map();
					std::memcpy(ptr, arg.data.data(), arg.data.size_bytes());
					staging_buffer.unmap();
				}
				// Schedule work to transfer.
				switch(res->get_type())
				{
					case resource_type::buffer:
					{
						vk2::Buffer& buffer = static_cast<buffer_component_vulkan*>(comp)->vk_get_buffer();
						this->command.do_scratch_operations([&buffer, &staging_buffer, &arg](vk2::CommandBufferRecording& record)
						{
							record.buffer_copy_buffer
							({
								.src = &staging_buffer,
								.dst = &buffer,
								.src_offset = 0,
								.dst_offset = arg.offset
							});
						});

					}
					break;
					case resource_type::image:
					{
						vk2::Image& image = static_cast<image_component_vulkan*>(comp)->vk_get_image();
						if(arg.offset != 0)
						{
							tz::report("renderer_edit::resource_write: Offset variable is detected to be %zu. Because the resource being written to is an image, this value has been ignored.", arg.offset);
						}
						vk2::ImageLayout cur_layout = image.get_layout();
						vk2::ImageAspectFlags aspect = vk2::derive_aspect_from_format(image.get_format());
						this->command.do_scratch_operations([&image, &staging_buffer, &cur_layout, aspect](vk2::CommandBufferRecording& record)
						{
							// Need to be transfer destination layout. After that we go back to what we were.	
							record.transition_image_layout
							({
								.image = &image,
								.target_layout = vk2::ImageLayout::TransferDestination,
								.source_access = {vk2::AccessFlag::NoneNeeded},
								.destination_access = {vk2::AccessFlag::TransferOperationWrite},
								.source_stage = vk2::PipelineStage::Top,
								.destination_stage = vk2::PipelineStage::TransferCommands,
								.image_aspects = aspect
							});

							record.buffer_copy_image
							({
								.src = &staging_buffer,
								.dst = &image,
								.image_aspects = aspect
							});

							record.transition_image_layout
							({
								.image = &image,
								.target_layout = cur_layout,
								.source_access = {vk2::AccessFlag::TransferOperationWrite} /*todo: correct values?*/,
								.destination_access = {vk2::AccessFlag::ShaderResourceRead},
								.source_stage = vk2::PipelineStage::TransferCommands,
								.destination_stage = vk2::PipelineStage::FragmentShader,
								.image_aspects = aspect
							});

						});
					}
					break;
				}
			}
			break;
			default:
			{
				tz::report("Received component write edit request for resource handle %zu, which is being carried out, but is unnecessary because the resource has dynamic access, meaning you can just mutate data().", static_cast<std::size_t>(static_cast<tz::hanval>(arg.resource)));
				std::span<std::byte> resdata = res->data_as<std::byte>();
				std::copy(arg.data.begin(), arg.data.end(), resdata.begin() + arg.offset);
			}
			break;
		}
	}

	void renderer_vulkan::edit_compute_config(renderer_edit::compute_config arg, EditData& data)
	{
		if(arg.kernel != this->state.compute.kernel)
		{
			this->state.compute.kernel = arg.kernel;
			data.commands_rerecord = true;
		}
	}

	void renderer_vulkan::edit_resource_reference(renderer_edit::resource_reference arg, EditData& data)
	{
		(void)arg; (void)data;
		tz::error("resource Reference re-seating is not yet implemented.");
	}

	void renderer_vulkan::setup_static_resources()
	{
		TZ_PROFZONE("Vulkan Frontend - renderer_vulkan Setup Static Resources", 0xFFAAAA00);
		// Create staging buffers for each buffer and texture resource, and then fill the data with the resource data.
		std::vector<buffer_component_vulkan*> buffer_components;
		std::vector<image_component_vulkan*> image_components;
		for(std::size_t i = 0; i < this->resource_count(); i++)
		{
			icomponent* icomp = this->get_component(static_cast<tz::hanval>(i));
			switch(icomp->get_resource()->get_type())
			{
				case resource_type::buffer:
					buffer_components.push_back(static_cast<buffer_component_vulkan*>(icomp));
				break;
				case resource_type::image:
					image_components.push_back(static_cast<image_component_vulkan*>(icomp));
				break;
				default:
					tz::error("Unknown resource_type.");
				break;
			}
		}

		// Staging buffers.
		std::vector<vk2::Buffer> staging_buffers;
		staging_buffers.reserve(buffer_components.size());
		std::vector<vk2::Buffer> staging_image_buffers;
		staging_image_buffers.reserve(image_components.size());
		// Fill buffers with resource data.
		std::transform(buffer_components.begin(), buffer_components.end(), std::back_inserter(staging_buffers),
		[](const buffer_component_vulkan* buf)->vk2::Buffer
		{
			return
			{{
				.device = &buf->vk_get_buffer().get_device(),
				.size_bytes = buf->size(),
				.usage = {vk2::BufferUsage::TransferSource},
				.residency = vk2::MemoryResidency::CPU
			}};
		});
		// Same with images.
		std::transform(image_components.begin(), image_components.end(), std::back_inserter(staging_image_buffers),
		[](const image_component_vulkan* img)->vk2::Buffer
		{
			return
			{{
				.device = &img->vk_get_image().get_device(),
				.size_bytes = pixel_size_bytes(img->get_format()) * img->get_dimensions()[0] * img->get_dimensions()[1],
				.usage = {vk2::BufferUsage::TransferSource},
				.residency = vk2::MemoryResidency::CPU
			}};
		});
		this->command.do_scratch_operations([&staging_buffers, &staging_image_buffers, &buffer_components, &image_components](vk2::CommandBufferRecording& recording)
		{
			// Finally, upload data for static resources.
			for(std::size_t i = 0; i < buffer_components.size(); i++)
			{
				iresource* res = buffer_components[i]->get_resource();
				tz::assert(res->get_type() == resource_type::buffer, "Expected resource_type of buffer, but is not a buffer. Please submit a bug report.");
				if(res->get_access() != resource_access::static_fixed)
				{
					continue;
				}

				// Now simply write the data straight in.
				{
					void* ptr = staging_buffers[i].map();
					std::memcpy(ptr, res->data().data(), res->data().size_bytes());
					staging_buffers[i].unmap();
				}

				// Record the command to transfer to the buffer resource.
				recording.buffer_copy_buffer
				({
					.src = &staging_buffers[i],
					.dst = &buffer_components[i]->vk_get_buffer()
				});
			}
			for(std::size_t i = 0; i < image_components.size(); i++)
			{
				iresource* res = image_components[i]->get_resource();
				tz::assert(res->get_type() == resource_type::image, "Expected resource_type of Texture, but is not a texture. Please submit a bug report.");
				if(res->get_access() != resource_access::static_fixed)
				{
					continue;
				}
				// Now simply write the data straight in.
				{
					void* ptr = staging_image_buffers[i].map();
					std::memcpy(ptr, res->data().data(), res->data().size_bytes());
					staging_image_buffers[i].unmap();
				}
				vk2::ImageAspectFlags aspect = vk2::derive_aspect_from_format(image_components[i]->vk_get_image().get_format());
				// Record the command to transfer to the texture resource.
				// Image will initially be in undefined layout. We need to:
				// - Transition the texture component to TransferDestination
				// - Transfer from the staging texture buffer
				// - Transition the texture component to ShaderResource so it can be used in the shader.
				recording.transition_image_layout
				({
					.image = &image_components[i]->vk_get_image(),
					.target_layout = vk2::ImageLayout::TransferDestination,
					.source_access = {vk2::AccessFlag::NoneNeeded},
					.destination_access = {vk2::AccessFlag::TransferOperationWrite},
					.source_stage = vk2::PipelineStage::Top,
					.destination_stage = vk2::PipelineStage::TransferCommands,
					.image_aspects = aspect
				});
				recording.buffer_copy_image
				({
					.src = &staging_image_buffers[i],
					.dst = &image_components[i]->vk_get_image(),
					.image_aspects = aspect
				});
				recording.transition_image_layout
				({
					.image = &image_components[i]->vk_get_image(),
					.target_layout = vk2::ImageLayout::ShaderResource,
					.source_access = {vk2::AccessFlag::TransferOperationWrite},
					.destination_access = {vk2::AccessFlag::ShaderResourceRead},
					.source_stage = vk2::PipelineStage::TransferCommands,
					.destination_stage = vk2::PipelineStage::FragmentShader,
					.image_aspects = aspect
				});
			}
		});
	}

	void renderer_vulkan::setup_render_commands()
	{
		TZ_PROFZONE("Vulkan Frontend - renderer_vulkan Setup Render Commands", 0xFFAAAA00);
		tz::assert(!this->pipeline.is_compute(), "Running render command recording path, but pipeline is a compute pipeline. Logic error, please submit a bug report.");

		this->command.set_rendering_commands([this](vk2::CommandBufferRecording& recording, std::size_t framebuffer_id)
		{
			tz::assert(framebuffer_id < this->output.get_output_framebuffers().size(), "Attempted to retrieve output framebuffer at index %zu, but there are only %zu framebuffers available. Please submit a bug report.", framebuffer_id, this->output.get_output_framebuffers().size());
			recording.debug_begin_label
			({
				.name = this->debug_name
			});

			{
				vk2::CommandBufferRecording::RenderPassRun run{this->output.get_output_framebuffers()[framebuffer_id], recording, this->state.graphics.clear_colour};
				recording.bind_pipeline
				({
					.pipeline = &this->pipeline.get_pipeline(),
				});
				tz::basic_list<const vk2::DescriptorSet*> sets;
				if(!this->resources.descriptor_empty())
				{
					std::span<const vk2::DescriptorSet> resource_sets = this->resources.get_descriptor_sets();
					sets.resize(resource_sets.size());
					std::transform(resource_sets.begin(), resource_sets.end(), sets.begin(), [](const vk2::DescriptorSet& set){return &set;});
					recording.bind_descriptor_sets
					({
						.pipeline_layout = &this->pipeline.get_pipeline().get_layout(),
						.context = this->pipeline.get_pipeline().get_context(),
						.descriptor_sets = sets,
						.first_set_id = 0
					});
				}

				tz::vec2ui offset{0u, 0u};
				tz::vec2ui extent = this->output.get_output_dimensions();

				if(this->get_output() != nullptr)
				{
					if(this->get_output()->scissor != scissor_region::null())
					{
						offset = this->get_output()->scissor.offset;
						extent = this->get_output()->scissor.extent;
					}
				}

				recording.set_scissor_dynamic
				({
					.offset = offset,
					.extent = extent
				});

				const icomponent* idx_buf = this->get_component(this->state.graphics.index_buffer);
				const icomponent* ind_buf_gen = this->get_component(this->state.graphics.draw_buffer);;
				const auto* ind_buf = static_cast<const buffer_component_vulkan*>(ind_buf_gen);
				if(idx_buf == nullptr)
				{
					if(ind_buf == nullptr)
					{
						recording.draw
						({
							.vertex_count = static_cast<std::uint32_t>(3 * this->state.graphics.tri_count),
							.instance_count = 1,
							.first_vertex = 0,
							.first_instance = 0
						});
					}
					else
					{
						if(this->options.contains(tz::gl::renderer_option::draw_indirect_count))
						{
							//tz::error("draw_indirect_count (non-indexed) is NYI sorry");
							recording.draw_indirect_count
							({
								.draw_indirect_buffer = &ind_buf->vk_get_buffer(),
								.max_draw_count = static_cast<std::uint32_t>(ind_buf->get_resource()->data().size_bytes() / sizeof(VkDrawIndirectCommand)),
								.stride = static_cast<std::uint32_t>(sizeof(VkDrawIndirectCommand))
							});
						}
						else
						{
							recording.draw_indirect
							({
								.draw_indirect_buffer = &ind_buf->vk_get_buffer(),
								.draw_count = static_cast<std::uint32_t>(ind_buf->get_resource()->data().size_bytes() / sizeof(VkDrawIndirectCommand)),
								.stride = static_cast<std::uint32_t>(sizeof(VkDrawIndirectCommand))
							});
						}
					}
				}
				else
				{
					recording.bind_index_buffer
					({
						.index_buffer = &static_cast<const buffer_component_vulkan*>(idx_buf)->vk_get_buffer()
					});
					if(ind_buf == nullptr)
					{
						recording.draw_indexed
						({
							.index_count = static_cast<std::uint32_t>(3 * this->state.graphics.tri_count)
						});
					}
					else
					{
						if(this->options.contains(tz::gl::renderer_option::draw_indirect_count))
						{
							tz::error("draw_indirect_count (indexed) is NYI sorry");
						}
						else
						{
							recording.draw_indexed_indirect
							({
								.draw_indirect_buffer = &ind_buf->vk_get_buffer(),
								.draw_count = static_cast<std::uint32_t>(ind_buf->get_resource()->data().size_bytes() / sizeof(VkDrawIndexedIndirectCommand)),
								.stride = static_cast<std::uint32_t>(sizeof(VkDrawIndexedIndirectCommand)),
								.offset = static_cast<VkDeviceSize>(0)
							});
						}
					}
				}
			}
			recording.debug_end_label({});
		});
	}
	
	void renderer_vulkan::setup_compute_commands()
	{
		TZ_PROFZONE("Vulkan Frontend - renderer_vulkan Setup Compute Commands", 0xFFAAAA00);
		tz::assert(this->pipeline.is_compute(), "Running compute command recording path, but pipeline is a graphics pipeline. Logic error, please submit a bug report.");

		this->command.set_rendering_commands([this](vk2::CommandBufferRecording& recording, std::size_t framebuffer_id)
		{
			tz::assert(framebuffer_id < this->output.get_output_framebuffers().size(), "Attempted to retrieve output framebuffer at index %zu, but there are only %zu framebuffers available. Please submit a bug report.", framebuffer_id, this->output.get_output_framebuffers().size());
			recording.debug_begin_label
			({
				.name = this->debug_name
			});
			recording.bind_pipeline
			({
				.pipeline = &this->pipeline.get_pipeline(),
			});
			tz::basic_list<const vk2::DescriptorSet*> sets;
			if(!this->resources.empty())
			{
				std::span<const vk2::DescriptorSet> resource_sets = this->resources.get_descriptor_sets();
				sets.resize(resource_sets.size());
				std::transform(resource_sets.begin(), resource_sets.end(), sets.begin(), [](const vk2::DescriptorSet& set){return &set;});
				recording.bind_descriptor_sets
				({
					.pipeline_layout = &this->pipeline.get_pipeline().get_layout(),
					.context = this->pipeline.get_pipeline().get_context(),
					.descriptor_sets = sets,
					.first_set_id = 0
				});
			}
			recording.dispatch
			({
				.groups = this->state.compute.kernel
			});

			recording.debug_end_label({});
		});
	}

	void renderer_vulkan::setup_work_commands()
	{
		if(this->pipeline.is_compute())
		{
			this->setup_compute_commands();
		}
		else
		{
			this->setup_render_commands();
		}
	}

	bool renderer_vulkan::handle_swapchain_outdated()
	{
		// Tell the DeviceWindow to update.
		tz::gl::get_device().get_device_window().request_refresh();
		auto& devwnd = tz::gl::get_device().get_device_window();
		this->handle_resize
		({
			.new_dimensions = devwnd.get_swapchain().get_dimensions(),
			.new_output_images = devwnd.get_output_images(),
			.new_depth_image = &devwnd.get_depth_image()
		});
		this->window_dims_cache = tz::window().get_dimensions();
		return this->present_failure_count <= 1;
	}

	void renderer_vulkan::handle_resize(const RendererResizeInfoVulkan& resize_info)
	{
		TZ_PROFZONE("Vulkan Frontend - renderer_vulkan Handle Resize", 0xFFAAAA00);
		// Context: The top-level gl::device has just been told by the window that it has been resized, and has recreated a new swapchain. Our old pointer to the swapchain `maybe_swapchain` correctly points to the new swapchain already, so we just have to recreate all the new state.
		this->command.wait_pending_commands_complete();
		this->output.create_output_resources(resize_info.new_output_images, resize_info.new_depth_image);
		this->pipeline.recreate(this->output.get_render_pass(), resize_info.new_dimensions, this->pipeline.is_wireframe_mode());
		this->setup_work_commands();
	}
}

#endif // TZ_VULKAN
