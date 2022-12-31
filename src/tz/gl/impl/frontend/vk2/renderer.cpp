#if TZ_VULKAN
#include "hdk/profile.hpp"
#include "hdk/debug.hpp"
#include "tz/dbgui/dbgui.hpp"
#include "tz/gl/declare/image_format.hpp"
#include "tz/gl/impl/frontend/vk2/device.hpp"
#include "tz/gl/impl/frontend/vk2/renderer.hpp"
#include "tz/gl/impl/frontend/vk2/component.hpp"
#include "tz/gl/impl/frontend/vk2/convert.hpp"
#include "tz/gl/impl/backend/vk2/sampler.hpp"
#include "tz/gl/impl/backend/vk2/tz_vulkan.hpp"
#include "tz/gl/impl/backend/vk2/fixed_function.hpp"
#include "tz/gl/impl/backend/vk2/gpu_mem.hpp"
#include "tz/gl/impl/backend/vk2/descriptors.hpp"
#include "tz/gl/impl/backend/vk2/image_view.hpp"
#include "tz/gl/output.hpp"

namespace tz::gl
{
	using namespace tz::gl;

//--------------------------------------------------------------------------------------------------
	// Utility free-functions.
	vk2::SamplerInfo make_fitting_sampler(const IResource& res)
	{
		vk2::LookupFilter filter = vk2::LookupFilter::Nearest;
		vk2::MipLookupFilter mip_filter = vk2::MipLookupFilter::Nearest;
		vk2::SamplerAddressMode mode = vk2::SamplerAddressMode::ClampToEdge;
#if HDK_DEBUG
		if(res.get_flags().contains({ResourceFlag::ImageFilterNearest, ResourceFlag::ImageFilterLinear}))
		{
			hdk::error("ImageResource contained both ResourceFlags ImageFilterNearest and ImageFilterLinear, which are mutually exclusive. Please submit a bug report.");
		}
#endif // HDK_DEBUG
		if(res.get_flags().contains(ResourceFlag::ImageFilterNearest))
		{
			filter = vk2::LookupFilter::Nearest;
		}
		else if(res.get_flags().contains(ResourceFlag::ImageFilterLinear))
		{
			filter = vk2::LookupFilter::Linear;
		}

		if(res.get_flags().contains({ResourceFlag::ImageWrapClampEdge, ResourceFlag::ImageWrapRepeat, ResourceFlag::ImageWrapMirroredRepeat}))
		{
			hdk::error("ResourceFlags included all 3 of ImageWrapClampEdge, ImageWrapRepeat and ImageWrapMirroredRepeat, all of which are mutually exclusive. Please submit a bug report.");
		}
		if(res.get_flags().contains(ResourceFlag::ImageWrapClampEdge))
		{
			mode = vk2::SamplerAddressMode::ClampToEdge;
		}
		if(res.get_flags().contains(ResourceFlag::ImageWrapRepeat))
		{
			mode = vk2::SamplerAddressMode::Repeat;
		}
		if(res.get_flags().contains(ResourceFlag::ImageWrapMirroredRepeat))
		{
			mode = vk2::SamplerAddressMode::MirroredRepeat;
		}
		
		return
		{
			.device = &tz::gl::device().vk_get_logical_device(),
			.min_filter = filter,
			.mag_filter = filter,
			.mipmap_mode = mip_filter,
			.address_mode_u = mode,
			.address_mode_v = mode,
			.address_mode_w = mode
		};
	}

//--------------------------------------------------------------------------------------------------
	ResourceStorage::ResourceStorage(const RendererInfoVulkan& info):
	AssetStorageCommon<IResource>(info.get_resources()),
	frame_in_flight_count(device().get_device_window().get_output_images().size())
	{
		HDK_PROFZONE("Vulkan Frontend - RendererVulkan ResourceStorage Create", 0xFFAAAA00);
		const vk2::LogicalDevice& ldev = device().vk_get_logical_device();
		this->samplers.reserve(this->count());

		auto resources = info.get_resources();
		std::size_t encountered_reference_count = 0;

		auto retrieve_resource_metadata = [this](IComponent* cmp)
		{
			IResource* res = cmp->get_resource();
			switch(res->get_type())
			{
				case ResourceType::Buffer:
				{
					// If the buffer is dynamic, let's link up the resource data span now.
					if(res->get_access() == ResourceAccess::DynamicFixed || res->get_access() == ResourceAccess::DynamicVariable)
					{
						std::span<const std::byte> initial_data = res->data();
						std::span<std::byte> buffer_byte_data = this->components.back().as<BufferComponentVulkan>()->vk_get_buffer().map_as<std::byte>();
						std::copy(initial_data.begin(), initial_data.end(), buffer_byte_data.begin());
						res->set_mapped_data(buffer_byte_data);
					}
				}
				break;
				case ResourceType::Image:
				{
					this->samplers.emplace_back(make_fitting_sampler(*res));

					auto* img = static_cast<ImageComponentVulkan*>(cmp);
					// We will need to create an image view. Let's get that out-of-the-way-now.
					vk2::Image& underlying_image = img->vk_get_image();
					this->image_component_views.emplace_back
						(vk2::ImageViewInfo{
							.image = &underlying_image,
							.aspect = vk2::derive_aspect_from_format(underlying_image.get_format()).front()
						 });
					// If the image is dynamic, let's link up the resource data span now.
					if(res->get_access() == ResourceAccess::DynamicFixed || res->get_access() == ResourceAccess::DynamicVariable)
					{

						std::span<const std::byte> initial_data = res->data();
						std::span<std::byte> image_data = {reinterpret_cast<std::byte*>(underlying_image.map()), initial_data.size_bytes()};
						std::copy(initial_data.begin(), initial_data.end(), image_data.begin());
						res->set_mapped_data(image_data);
					}
				}
				break;
				default:
					hdk::error("Unrecognised ResourceType. Please submit a bug report.");
				break;
			}
		};

		this->components.reserve(this->count());
		for(std::size_t i = 0; i < this->count(); i++)
		{
			IResource* res = this->get(static_cast<hdk::hanval>(i));
			IComponent* comp = nullptr;
			if(res == nullptr)
			{
				// If we see a null resource, it means we're looking for a component (resource reference).
				comp = const_cast<IComponent*>(info.get_components()[encountered_reference_count]);
				this->components.push_back(comp);
				encountered_reference_count++;

				res = comp->get_resource();
				// Also we'll write into the asset storage so it doesn't still think the resource is null.
				this->set(static_cast<hdk::hanval>(i), res);
			}
			else
			{
				switch(res->get_type())
				{
					case ResourceType::Buffer:
					{
						this->components.push_back(tz::make_owned<BufferComponentVulkan>(*res));
					}
					break;
					case ResourceType::Image:
						this->components.push_back(tz::make_owned<ImageComponentVulkan>(*res));
					break;
					default:
						hdk::error("Unrecognised ResourceType. Please submit a bug report.");
					break;
				}
				comp = this->components.back().get();
			}
			retrieve_resource_metadata(comp);
		}

		std::size_t descriptor_buffer_count = this->resource_count_of(ResourceType::Buffer);
		if(info.state().graphics.index_buffer != hdk::nullhand)
		{
			descriptor_buffer_count--;
		}
		if(info.state().graphics.draw_buffer != hdk::nullhand)
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
			if(this->resource_count_of(ResourceType::Image))
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
			tz::BasicList<const vk2::DescriptorLayout*> alloc_layout_list;
			for(std::size_t i = 0; i < this->frame_in_flight_count; i++)
			{
				alloc_layout_list.add(&this->descriptor_layout);
			}
			this->descriptors = this->descriptor_pool.allocate_sets
			({
				.set_layouts = std::move(alloc_layout_list)
			});
		};
		hdk::assert(this->descriptors.success(), "Descriptor Pool allocation failed. Please submit a bug report.");
		this->sync_descriptors(true, info.state());
	}

	ResourceStorage::ResourceStorage(ResourceStorage&& move):
	AssetStorageCommon<IResource>(static_cast<AssetStorageCommon<IResource>&&>(move)),
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
		AssetStorageCommon<IResource>::operator=(std::move(rhs));
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
			

	const IComponent* ResourceStorage::get_component(ResourceHandle handle) const
	{
		if(handle == hdk::nullhand)
		{
			return nullptr;
		}
		return this->components[static_cast<std::size_t>(static_cast<hdk::hanval>(handle))].get();
	}

	IComponent* ResourceStorage::get_component(ResourceHandle handle)
	{
		if(handle == hdk::nullhand)
		{
			return nullptr;
		}
		return this->components[static_cast<std::size_t>(static_cast<hdk::hanval>(handle))].get();
	}

	const vk2::DescriptorLayout& ResourceStorage::get_descriptor_layout() const
	{
		return this->descriptor_layout;
	}

	std::span<const vk2::DescriptorSet> ResourceStorage::get_descriptor_sets() const
	{
		return this->descriptors.sets;
	}

	std::size_t ResourceStorage::resource_count_of(ResourceType type) const
	{
		return std::count_if(this->components.begin(), this->components.end(),
		[type](const auto& component_ptr)
		{
			return component_ptr->get_resource()->get_type() == type;
		});
	}

	void ResourceStorage::notify_image_recreated(ResourceHandle image_resource_handle)
	{
		// ImageComponent's underlying vk2::Image was recently replaced with another. This means this->image_component_views[id corresponding to handle] is wrong and needs to be remade.
		std::size_t img_view_idx = 0;
		auto handle_val = static_cast<std::size_t>(static_cast<hdk::hanval>(image_resource_handle));
		for(std::size_t i = 0; i < handle_val; i++)
		{
			if(this->get(static_cast<hdk::hanval>(i))->get_type() == ResourceType::Image)
			{
				img_view_idx++;
			}
		}
		auto& img = static_cast<ImageComponentVulkan*>(this->get_component(image_resource_handle))->vk_get_image();
		this->image_component_views[img_view_idx] =
		{{
			.image = &img,
			.aspect = vk2::derive_aspect_from_format(img.get_format()).front()
		}};
	}

	void ResourceStorage::sync_descriptors(bool write_everything, const RenderState& state)
	{
		HDK_PROFZONE("Vulkan Frontend - RendererVulkan ResourceStorage Descriptor Sync", 0xFFAAAA00);
		std::vector<BufferComponentVulkan*> buffers;
		buffers.reserve(this->components.size());
		for(auto& component_ptr : this->components)
		{
			if(component_ptr->get_resource()->get_type() == ResourceType::Buffer)
			{
				buffers.push_back(component_ptr.as<BufferComponentVulkan>());
			}
		}
		std::size_t descriptor_buffer_count = buffers.size();
		if(state.graphics.index_buffer != hdk::nullhand)
		{
			descriptor_buffer_count--;
		}
		if(state.graphics.draw_buffer != hdk::nullhand)
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
				BufferComponentVulkan& comp = *buffers[j];
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
			IResource* res = component_ptr->get_resource();
			if(res->get_type() == ResourceType::Image && res->get_access() != ResourceAccess::StaticFixed)
			{
				const vk2::Image& img = component_ptr.as<const ImageComponentVulkan>()->vk_get_image();
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
				hdk::assert(row_pitch > resource_row_pitch, "Linear CPU vk2::Image row data is negatively-padded? Assuming ImageResource data is tightly-packed, it uses %zu bytes per row, but the actual image uses %zu? It should be using more, not less!", resource_row_pitch, row_pitch);
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

	OutputManager::OutputManager(const RendererInfoVulkan& info):
	output(info.get_output() != nullptr ? info.get_output()->unique_clone() : nullptr),
	ldev(&device().vk_get_logical_device()),
	swapchain_images(device().get_device_window().get_output_images()),
	swapchain_depth_images(&device().get_device_window().get_depth_image()),
	options(info.get_options())
	{
		HDK_PROFZONE("Vulkan Frontend - RendererVulkan OutputManager Create", 0xFFAAAA00);
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
		if(this->output == nullptr || this->output->get_target() == OutputTarget::Window)
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
		else if(this->output->get_target() == OutputTarget::OffscreenImage)
		{
			// We have been provided an ImageOutput which will contain an ImageComponentVulkan. We need to retrieve that image and return a span covering it.
			auto& out = static_cast<ImageOutput&>(*this->output);

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
				// Now we weren't given a depth image. We should leave this nullptr, but only if NoDepthTesting was enabled. If we need to do depth testing but weren't given a depth image, we'll use the swapchain depth image.
				if(this->options.contains(RendererOption::NoDepthTesting))
				{
					out_image.depth_attachment = nullptr;
				}
				else
				{
					out_image.depth_attachment = this->swapchain_depth_images;
				}
			}

			std::vector<OutputImageState> ret(this->swapchain_images.size(), out_image);
			hdk::assert(!out.has_depth_attachment(), "Depth attachment on an ImageOutput is not yet implemented");
			return ret;
		}
		else
		{
			hdk::error("Unrecognised OutputTarget. Please submit a bug report.");
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

	hdk::vec2ui OutputManager::get_output_dimensions() const
	{
		hdk::assert(!this->output_imageviews.empty(), "OutputManager had no output views, so impossible to retrieve viewport dimensions. Please submit a bug report.");
		return this->output_imageviews.front().colour_views.front().get_image().get_dimensions();
	}

	IOutput* OutputManager::get_output()
	{
		return this->output.get();
	}

	const IOutput* OutputManager::get_output() const
	{
		return this->output.get();
	}
	
	bool OutputManager::has_depth_images() const
	{
		return !this->options.contains(RendererOption::NoDepthTesting);
	}

	void OutputManager::create_output_resources(std::span<vk2::Image> swapchain_images, vk2::Image* depth_image)
	{
		HDK_PROFZONE("Vulkan Frontend - RendererVulkan OutputManager (Output Resources Creation)", 0xFFAAAA00);
		this->swapchain_images = swapchain_images;
		this->output_imageviews.clear();
		this->output_depth_imageviews.clear();
		this->output_framebuffers.clear();

		this->swapchain_depth_images = depth_image;
		this->output_depth_imageviews.reserve(this->swapchain_images.size());

		this->populate_output_views();

		#if HDK_DEBUG
			for(const OutputImageViewState& out_view : this->output_imageviews)
			{
				hdk::assert(std::equal(out_view.colour_views.begin(), out_view.colour_views.end(), out_view.colour_views.begin(), [](const vk2::ImageView& a, const vk2::ImageView& b){return a.get_image().get_format() == b.get_image().get_format();}), "Detected that not every output image in a RendererVulkan has the same format. This is not permitted as RenderPasses would not be compatible. Please submit a bug report.");
			}
		#endif // HDK_DEBUG

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
		// We're matching the ImageFormat of the provided output image.
		vk2::ImageLayout final_layout;
		if(this->output == nullptr || this->output->get_target() == OutputTarget::Window)
		{
			final_layout = vk2::ImageLayout::Present;
		}
		else if(this->output->get_target() == OutputTarget::OffscreenImage)
		{
			final_layout = vk2::ImageLayout::ShaderResource;
		}
		else
		{
			final_layout = vk2::ImageLayout::Undefined;
			hdk::error("Unknown RendererOutputType. Please submit a bug report.");
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
				.colour_depth_load = this->options.contains(RendererOption::NoClearOutput) ? vk2::LoadOp::Load : vk2::LoadOp::Clear,
				.colour_depth_store = vk2::StoreOp::Store,
				.initial_layout = this->options.contains(RendererOption::NoClearOutput) ? vk2::ImageLayout::Present : vk2::ImageLayout::Undefined,
				.final_layout = final_layout
			});
		}

		if(use_depth_output)
		{
			rbuilder.with_attachment
			({
				.format = output_image_copy.front().depth_attachment->get_format(),
				.colour_depth_load = this->options.contains(RendererOption::NoClearOutput) ? vk2::LoadOp::Load : vk2::LoadOp::Clear,
				.colour_depth_store = this->options.contains(RendererOption::NoPresent) ? vk2::StoreOp::Store : vk2::StoreOp::DontCare,
				.initial_layout = this->options.contains(RendererOption::NoClearOutput) ? vk2::ImageLayout::DepthStencilAttachment : vk2::ImageLayout::Undefined,
				.final_layout = vk2::ImageLayout::DepthStencilAttachment
			});
		}

		vk2::SubpassBuilder sbuilder;
		sbuilder.set_pipeline_context(vk2::PipelineContext::Graphics);
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
			hdk::assert(this->output_imageviews.size() == this->output_depth_imageviews.size(), "Detected at least 1 output depth image views (%zu), but that doesn't match the number of output colour imageviews (%zu). Please submit a bug report", this->output_depth_imageviews.size(), this->output_imageviews.size());
		}
		for(std::size_t i = 0; i < this->output_imageviews.size(); i++)
		{
			hdk::vec2ui dims = this->output_imageviews[i].colour_views.front().get_image().get_dimensions();
			tz::BasicList<vk2::ImageView*> attachments;
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

	GraphicsPipelineManager::GraphicsPipelineManager(const RendererInfoVulkan& info, const ResourceStorage& resources, const OutputManager& output)
	{
		this->shader = this->make_shader(device().vk_get_logical_device(), info.shader());
		this->pipeline_layout = this->make_pipeline_layout(resources.get_descriptor_layout(), device().get_device_window().get_output_images().size());
		this->depth_testing_enabled = !info.get_options().contains(RendererOption::NoDepthTesting);
		const bool alpha_blending_enabled = info.get_options().contains(RendererOption::AlphaBlending);
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

	void GraphicsPipelineManager::recreate(const vk2::RenderPass& new_render_pass, hdk::vec2ui new_viewport_dimensions, bool wireframe_mode)
	{
		this->wireframe_mode = wireframe_mode;
		HDK_PROFZONE("Vulkan Frontend - RendererVulkan GraphicsPipelineManager Recreate", 0xFFAAAA00);
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
					.viewport = vk2::create_basic_viewport(static_cast<hdk::vec2>(new_viewport_dimensions)),
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

	vk2::Shader GraphicsPipelineManager::make_shader(const vk2::LogicalDevice& ldev, const ShaderInfo& sinfo) const
	{
		HDK_PROFZONE("Vulkan Frontend - RendererVulkan GraphicsPipelineManager (Shader Create)", 0xFFAAAA00);
		std::vector<char> vtx_src, frg_src, tesscon_src, tesseval_src, cmp_src;
		tz::BasicList<vk2::ShaderModuleInfo> modules;
		if(sinfo.has_shader(ShaderStage::Compute))
		{
			hdk::assert(!sinfo.has_shader(ShaderStage::Vertex), "Shader has compute shader and vertex shader. These are mutually exclusive.");
			hdk::assert(!sinfo.has_shader(ShaderStage::Fragment), "Shader has compute shader and fragment shader. These are mutually exclusive.");
			// Compute, we only care about the compute shader.
			{
				std::string_view compute_source = sinfo.get_shader(ShaderStage::Compute);
				cmp_src.resize(compute_source.length());
				std::copy(compute_source.begin(), compute_source.end(), cmp_src.begin());
			}
			modules =
			{
				{
					.device = &ldev,
					.type = vk2::ShaderType::Compute,
					.code = cmp_src
				}
			};
		}
		else
		{
			// Graphics, must contain a Vertex and Fragment shader.
			hdk::assert(sinfo.has_shader(ShaderStage::Vertex), "ShaderInfo must contain a non-empty vertex shader if no compute shader is present.");
			hdk::assert(sinfo.has_shader(ShaderStage::Fragment), "ShaderInfo must contain a non-empty fragment shader if no compute shader is present.");
			{
				std::string_view vertex_source = sinfo.get_shader(ShaderStage::Vertex);
				vtx_src.resize(vertex_source.length());
				std::copy(vertex_source.begin(), vertex_source.end(), vtx_src.begin());

				std::string_view fragment_source = sinfo.get_shader(ShaderStage::Fragment);
				frg_src.resize(fragment_source.length());
				std::copy(fragment_source.begin(), fragment_source.end(), frg_src.begin());
			}
			modules = 
			{
				{
					.device = &ldev,
					.type = vk2::ShaderType::Vertex,
					.code = vtx_src
				},
				{
					.device = &ldev,
					.type = vk2::ShaderType::Fragment,
					.code = frg_src
				}
			};
			// Add optional shader stages.
			if(sinfo.has_shader(ShaderStage::TessellationControl) || sinfo.has_shader(ShaderStage::TessellationEvaluation))
			{
				hdk::assert(sinfo.has_shader(ShaderStage::TessellationControl) && sinfo.has_shader(ShaderStage::TessellationEvaluation), "Detected a tessellaton shader type, but it was missing its sister. If a control or evaluation shader exists, they both must exist.");

				std::string_view tesscon_source = sinfo.get_shader(ShaderStage::TessellationControl);
				tesscon_src.resize(tesscon_source.length());
				std::copy(tesscon_source.begin(), tesscon_source.end(), tesscon_src.begin());

				std::string_view tesseval_source = sinfo.get_shader(ShaderStage::TessellationEvaluation);
				tesseval_src.resize(tesseval_source.length());
				std::copy(tesseval_source.begin(), tesseval_source.end(), tesseval_src.begin());
				modules.add(vk2::ShaderModuleInfo
				{
					.device = &ldev,
					.type = vk2::ShaderType::TessellationControl,
					.code = tesscon_src
				});
				modules.add(vk2::ShaderModuleInfo
				{
					.device = &ldev,
					.type = vk2::ShaderType::TessellationEvaluation,
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

	vk2::GraphicsPipelineInfo GraphicsPipelineManager::make_graphics_pipeline(hdk::vec2ui viewport_dimensions, bool depth_testing_enabled, bool alpha_blending_enabled, const vk2::RenderPass& render_pass) const
	{
		tz::BasicList<vk2::ColourBlendState::AttachmentState> alpha_blending_options;
		alpha_blending_options.resize(render_pass.get_info().total_colour_attachment_count());
		std::fill(alpha_blending_options.begin(), alpha_blending_options.end(), alpha_blending_enabled ? vk2::ColourBlendState::alpha_blending() : vk2::ColourBlendState::no_blending());
		return
		{
			.shaders = this->shader.native_data(),
			.state =
			{
				.viewport = vk2::create_basic_viewport(static_cast<hdk::vec2>(viewport_dimensions)),
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

	vk2::Pipeline GraphicsPipelineManager::make_pipeline(hdk::vec2ui viewport_dimensions, bool depth_testing_enabled, bool alpha_blending_enabled, const vk2::RenderPass& render_pass) const
	{
		if(this->is_compute())
		{
			return {this->make_compute_pipeline()};
		}

		return {this->make_graphics_pipeline(viewport_dimensions, depth_testing_enabled, alpha_blending_enabled, render_pass)};
	}

//--------------------------------------------------------------------------------------------------

	CommandProcessor::CommandProcessor(const RendererInfoVulkan& info)
	{
		if(info.get_output() == nullptr || info.get_output()->get_target() == OutputTarget::Window)
		{
			this->requires_present = true;
		}
		this->instant_compute_enabled = info.get_options().contains(RendererOption::RenderWait);
		this->graphics_queue = device().vk_get_logical_device().get_hardware_queue
		({
			.field = {vk2::QueueFamilyType::Graphics},
			.present_support = this->requires_present
		});
		this->compute_queue = device().vk_get_logical_device().get_hardware_queue
		({
			.field = {vk2::QueueFamilyType::Compute},
			.present_support = false
		});
		this->command_pool = vk2::CommandPool{{.queue = this->graphics_queue, .flags = {vk2::CommandPoolFlag::Reusable}}};
		this->frame_in_flight_count = device().get_device_window().get_output_images().size();
		this->commands = this->command_pool.allocate_buffers
		({
			.buffer_count = static_cast<std::uint32_t>(this->frame_in_flight_count + 1)
		});
		this->images_in_flight.resize(this->frame_in_flight_count, nullptr);
		this->options = info.get_options();
		this->device_scheduler = &device().get_render_scheduler();

		hdk::assert(this->graphics_queue != nullptr, "Could not retrieve graphics present queue. Either your machine does not meet requirements, or (more likely) a logical error. Please submit a bug report.");
		hdk::assert(this->compute_queue != nullptr, "Could not retrieve compute queue. Either your machine does not meet requirements, or (more likely) a logical error. Please submit a bug report.");
		hdk::assert(this->commands.success(), "Failed to allocate from CommandPool");
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
		HDK_PROFZONE("Vulkan Frontend - RendererVulkan CommandProcessor (Do Render Work)", 0xFFAAAA00);
		auto& device_window = device().get_device_window();
		// Submit & Present
		this->device_scheduler->get_frame_fences()[this->current_frame].wait_until_signalled();
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
				target_image->wait_until_signalled();
			}
			target_image = &this->device_scheduler->get_frame_fences()[this->output_image_index];
		}
		else
		{
			this->output_image_index = this->current_frame;
		}

		this->device_scheduler->get_frame_fences()[this->current_frame].unsignal();
		tz::BasicList<vk2::hardware::Queue::SubmitInfo::WaitInfo> waits;
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
		tz::BasicList<const vk2::BinarySemaphore*> sem_signals;
		if(requires_present && !this->options.contains(RendererOption::NoPresent))
		{
			sem_signals = {&this->device_scheduler->get_render_work_signals()[this->current_frame]};
		}
		this->graphics_queue->submit
		({
			.command_buffers = {&this->get_render_command_buffers()[this->output_image_index]},
			.waits = waits,
			.signal_semaphores = sem_signals,
			.execution_complete_fence = &this->device_scheduler->get_frame_fences()[this->current_frame]
		});

		if(this->instant_compute_enabled)
		{
			this->device_scheduler->get_frame_fences()[this->current_frame].wait_until_signalled();
		}

		CommandProcessor::RenderWorkSubmitResult result;

		if(requires_present && !this->options.contains(RendererOption::NoPresent))
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
			result.present = vk2::hardware::Queue::PresentResult::Success;
		}

		this->current_frame = (this->current_frame + 1) % this->frame_in_flight_count;
		return result;
	}

	void CommandProcessor::do_compute_work()
	{
		HDK_PROFZONE("Vulkan Frontend - RendererVulkan CommandProcessor (Do Compute Work)", 0xFFAAAA00);

		this->device_scheduler->get_frame_fences()[this->current_frame].wait_until_signalled();
		this->device_scheduler->get_frame_fences()[this->current_frame].unsignal();
		this->compute_queue->submit
		({
			.command_buffers = {&this->get_render_command_buffers().front()},
			.waits = {},
			.signal_semaphores = {},
			.execution_complete_fence = &this->device_scheduler->get_frame_fences()[this->current_frame]
		});

		if(this->instant_compute_enabled)
		{
			this->device_scheduler->get_frame_fences()[this->current_frame].wait_until_signalled();
		}
	}

	void CommandProcessor::wait_pending_commands_complete()
	{
		HDK_PROFZONE("Vulkan Frontend - RendererVulkan CommandProcessor (Waiting on commands to complete)", 0xFFAAAA00);
		this->device_scheduler->wait_frame_work_complete();
	}

//--------------------------------------------------------------------------------------------------

	RendererVulkan::RendererVulkan(const RendererInfoVulkan& info):
	ldev(&device().vk_get_logical_device()),
	options(info.get_options()),
	state(info.state()),
	resources(info),
	output(info),
	pipeline(info, resources, output),
	command(info),
	debug_name(info.debug_get_name())
	{
		HDK_PROFZONE("Vulkan Frontend - RendererVulkan Create", 0xFFAAAA00);

		//	By default, we don't need to know about resizes.	
		bool care_about_resizes = false;
		// If we render into a window, or there is no output, then we need to know about resizes.
		if(info.get_output() == nullptr || info.get_output()->get_target() == OutputTarget::Window)
		{
			care_about_resizes = true;
		}
		// If instead we're rendering into an image, and depth testing is enabled, *but* there is no depth image provided, then we're gonna use the device's depth image which can be resized, and thus we care about resizes.
		else if(info.get_output() != nullptr && info.get_output()->get_target() == OutputTarget::OffscreenImage && !static_cast<const ImageOutput*>(info.get_output())->has_depth_attachment() && !info.get_options().contains(RendererOption::NoDepthTesting))
		{
			care_about_resizes = true;
		}
		
		if(care_about_resizes)
		{
			this->window_resize_callback = device().get_device_window().resize_callback().add_callback([this](RendererResizeInfoVulkan resize_info){this->handle_resize(resize_info);});
		}

		this->setup_static_resources();
		this->setup_work_commands();

		// Debug name settings.
		#if HDK_DEBUG
			for(std::size_t i = 0; i < this->resource_count(); i++)
			{
				IComponent* comp = this->resources.get_component(static_cast<hdk::hanval>(i));
				if(comp->get_resource()->get_type() == ResourceType::Buffer)
				{
					vk2::Buffer& buf = static_cast<BufferComponentVulkan*>(comp)->vk_get_buffer();
					std::string n = buf.debug_get_name();
					buf.debug_set_name(n + (n.empty() ? "" : " -> ") + this->debug_name + ":B" + std::to_string(i));
				}
				if(comp->get_resource()->get_type() == ResourceType::Image)
				{
					vk2::Image& img = static_cast<ImageComponentVulkan*>(comp)->vk_get_image();
					std::string n = img.debug_get_name();
					img.debug_set_name(n + (n.empty() ? "" : " -> ") + this->debug_name + ":I" + std::to_string(i));
				}
			}

			this->pipeline.get_shader().debug_set_name(this->debug_name);
		#endif
	}

	RendererVulkan::RendererVulkan(RendererVulkan&& move):
	ldev(move.ldev),
	options(move.options),
	resources(std::move(move.resources)),
	output(std::move(move.output)),
	pipeline(std::move(move.pipeline)),
	command(std::move(move.command)),
	debug_name(std::move(move.debug_name)),
	window_resize_callback(move.window_resize_callback),
	scissor_cache(move.scissor_cache)
	{
		auto& callback = device().get_device_window().resize_callback();
		
		if(this->window_resize_callback != hdk::nullhand)
		{
			callback.remove_callback(this->window_resize_callback);
		}
		this->window_resize_callback = callback.add_callback([this](RendererResizeInfoVulkan resize_info){this->handle_resize(resize_info);});
	}

	RendererVulkan::~RendererVulkan()
	{
		if(this->is_null())
		{
			return;
		}
		if(this->window_resize_callback != hdk::nullhand)
		{
			device().get_device_window().resize_callback().remove_callback(this->window_resize_callback);
			this->window_resize_callback = hdk::nullhand;
		}
		this->ldev->wait_until_idle();
	}

	RendererVulkan& RendererVulkan::operator=(RendererVulkan&& rhs)
	{
		std::swap(this->ldev, rhs.ldev);
		std::swap(this->options, rhs.options);
		std::swap(this->state, rhs.state);
		std::swap(this->resources, rhs.resources);
		std::swap(this->output, rhs.output);
		std::swap(this->pipeline, rhs.pipeline);
		std::swap(this->command, rhs.command);
		std::swap(this->debug_name, rhs.debug_name);
		std::swap(this->window_resize_callback, rhs.window_resize_callback);
		std::swap(this->scissor_cache, rhs.scissor_cache);
		auto& callback = device().get_device_window().resize_callback();
		if(this->is_null() && rhs.is_null())
		{

		}
		else if(this->is_null() && !rhs.is_null())
		{
			hdk::assert(this->window_resize_callback == hdk::nullhand);
			callback.remove_callback(rhs.window_resize_callback);
		}
		else if(!this->is_null() && rhs.is_null())
		{
			hdk::assert(rhs.window_resize_callback == hdk::nullhand);
			callback.remove_callback(this->window_resize_callback);
			this->window_resize_callback = callback.add_callback([this](RendererResizeInfoVulkan resize_info){this->handle_resize(resize_info);});
		}
		else
		{
			callback.remove_callback(rhs.window_resize_callback);
			callback.remove_callback(this->window_resize_callback);
			this->window_resize_callback = callback.add_callback([this](RendererResizeInfoVulkan resize_info){this->handle_resize(resize_info);});
		}
		return *this;
	}

	unsigned int RendererVulkan::resource_count() const
	{
		return this->resources.count();
	}

	const IResource* RendererVulkan::get_resource(ResourceHandle handle) const
	{
		return this->resources.get(handle);
	}

	IResource* RendererVulkan::get_resource(ResourceHandle handle)
	{
		return this->resources.get(handle);
	}

	const IComponent* RendererVulkan::get_component(ResourceHandle handle) const
	{
		return this->resources.get_component(handle);
	}

	IComponent* RendererVulkan::get_component(ResourceHandle handle)
	{
		return this->resources.get_component(handle);
	}

	IOutput* RendererVulkan::get_output()
	{
		return this->output.get_output();
	}

	const IOutput* RendererVulkan::get_output() const
	{
		return this->output.get_output();
	}

	const RendererOptions& RendererVulkan::get_options() const
	{
		return this->options;
	}

	const RenderState& RendererVulkan::get_state() const
	{
		return this->state;
	}

	void RendererVulkan::render()
	{
		HDK_PROFZONE("Vulkan Frontend - RendererVulkan Render", 0xFFAAAA00);
		
		// If output scissor region has changed, we need to rerecord.
		if(this->get_output() != nullptr)
		{
			HDK_PROFZONE("Vulkan Frontend - RendererVulkan Scissor Cache Miss", 0xFFAA0000);
			if(this->get_output()->scissor != this->scissor_cache)
			{
				this->scissor_cache = this->get_output()->scissor;
				this->command.wait_pending_commands_complete();
				this->setup_work_commands();
			}
		}
		this->resources.write_padded_image_data();
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
				case vk2::hardware::Queue::PresentResult::Success:

				break;
				default:
					hdk::error("Presentation failed, but for unknown reason. Please submit a bug report.");
				break;
			}
		}
	}

	void RendererVulkan::render(unsigned int tri_count)
	{
		if(this->state.graphics.tri_count != tri_count)
		{
			this->state.graphics.tri_count = tri_count;
			this->command.wait_pending_commands_complete();
			this->setup_work_commands();
		}
		this->render();
	}

	void RendererVulkan::edit(const RendererEditRequest& edit_request)
	{
		HDK_PROFZONE("Vulkan Frontend - RendererVulkan Edit", 0xFFAAAA00);

		bool final_wireframe_mode_state = this->pipeline.is_wireframe_mode();
		if(edit_request.empty())
		{
			return;
		}
		RendererVulkan::EditData data;
		this->command.wait_pending_commands_complete();
		for(const RendererEdit::Variant& req : edit_request)
		{
			std::visit([this, &data, &final_wireframe_mode_state](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;
				if constexpr(std::is_same_v<T, RendererEdit::BufferResize>)
				{
					  this->edit_buffer_resize(arg, data);
				}
				else if constexpr(std::is_same_v<T, RendererEdit::ImageResize>)
				{
					this->edit_image_resize(arg, data);
				}
				else if constexpr(std::is_same_v<T, RendererEdit::ResourceWrite>)
				{
					this->edit_resource_write(arg, data);
				}
				else if constexpr(std::is_same_v<T, RendererEdit::ComputeConfig>)
				{
					this->edit_compute_config(arg, data);
				}
				else if constexpr(std::is_same_v<T, RendererEdit::RenderConfig>)
				{
					if(arg.wireframe_mode != this->pipeline.is_wireframe_mode())
					{
						data.pipeline_recreate = true;
						final_wireframe_mode_state = arg.wireframe_mode;
					}
				}
				else if constexpr(std::is_same_v<T, RendererEdit::ResourceReference>)
				{
					this->edit_resource_reference(arg, data);
				}
				else
				{
					hdk::error("Renderer Edit that is not yet supported has been requested. Please submit a bug report.");
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

	void RendererVulkan::dbgui()
	{
		common_renderer_dbgui(*this);
	}

	std::string_view RendererVulkan::debug_get_name() const
	{
		return this->debug_name;
	}

	RendererVulkan RendererVulkan::null()
	{
		return {};
	}
	
	bool RendererVulkan::is_null() const
	{
		return this->ldev == nullptr;
	}

	void RendererVulkan::edit_buffer_resize(RendererEdit::BufferResize arg, EditData& data)
	{
		auto bufcomp = static_cast<BufferComponentVulkan*>(this->get_component(arg.buffer_handle));
		hdk::assert(bufcomp != nullptr, "Invalid buffer handle in RendererEdit::BufferResize");
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

	void RendererVulkan::edit_image_resize(RendererEdit::ImageResize arg, EditData& data)
	{
		auto imgcomp = static_cast<ImageComponentVulkan*>(this->get_component(arg.image_handle));
		hdk::assert(imgcomp != nullptr, "Invalid image handle in RendererEdit::ImageResize");
		if(imgcomp->get_dimensions() != arg.dimensions)
		{
			imgcomp->resize(arg.dimensions);
			// An imageview was looking at the old image, let's update that.
			this->resources.notify_image_recreated(arg.image_handle);
			// New image so descriptor array needs to be re-written to.
			data.descriptor_resync |= EditData::descriptor_resync_full;
		}
	}
	
	void RendererVulkan::edit_resource_write(RendererEdit::ResourceWrite arg, [[maybe_unused]] EditData& data)
	{
		IComponent* comp = this->get_component(arg.resource);
		IResource* res = comp->get_resource();
		// Note: Resource data won't change even though we change the buffer/image component. We need to set that aswell!
		switch(res->get_access())
		{
			case ResourceAccess::StaticFixed:
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
					case ResourceType::Buffer:
					{
						vk2::Buffer& buffer = static_cast<BufferComponentVulkan*>(comp)->vk_get_buffer();
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
					case ResourceType::Image:
					{
						vk2::Image& image = static_cast<ImageComponentVulkan*>(comp)->vk_get_image();
						if(arg.offset != 0)
						{
							hdk::report("RendererEdit::ResourceWrite: Offset variable is detected to be %zu. Because the resource being written to is an image, this value has been ignored.", arg.offset);
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
								.source_access = {vk2::AccessFlag::None},
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
				hdk::report("Received component write edit request for resource handle %zu, which is being carried out, but is unnecessary because the resource has dynamic access, meaning you can just mutate data().", static_cast<std::size_t>(static_cast<hdk::hanval>(arg.resource)));
				std::span<std::byte> resdata = res->data_as<std::byte>();
				std::copy(arg.data.begin(), arg.data.end(), resdata.begin() + arg.offset);
			}
			break;
		}
	}

	void RendererVulkan::edit_compute_config(RendererEdit::ComputeConfig arg, EditData& data)
	{
		if(arg.kernel != this->state.compute.kernel)
		{
			this->state.compute.kernel = arg.kernel;
			data.commands_rerecord = true;
		}
	}

	void RendererVulkan::edit_resource_reference(RendererEdit::ResourceReference arg, EditData& data)
	{
		(void)arg; (void)data;
		hdk::error("Resource Reference re-seating is not yet implemented.");
	}

	void RendererVulkan::setup_static_resources()
	{
		HDK_PROFZONE("Vulkan Frontend - RendererVulkan Setup Static Resources", 0xFFAAAA00);
		// Create staging buffers for each buffer and texture resource, and then fill the data with the resource data.
		std::vector<BufferComponentVulkan*> buffer_components;
		std::vector<ImageComponentVulkan*> image_components;
		for(std::size_t i = 0; i < this->resource_count(); i++)
		{
			IComponent* icomp = this->get_component(static_cast<hdk::hanval>(i));
			switch(icomp->get_resource()->get_type())
			{
				case ResourceType::Buffer:
					buffer_components.push_back(static_cast<BufferComponentVulkan*>(icomp));
				break;
				case ResourceType::Image:
					image_components.push_back(static_cast<ImageComponentVulkan*>(icomp));
				break;
				default:
					hdk::error("Unknown ResourceType.");
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
		[](const BufferComponentVulkan* buf)->vk2::Buffer
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
		[](const ImageComponentVulkan* img)->vk2::Buffer
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
				IResource* res = buffer_components[i]->get_resource();
				hdk::assert(res->get_type() == ResourceType::Buffer, "Expected ResourceType of buffer, but is not a buffer. Please submit a bug report.");
				if(res->get_access() != ResourceAccess::StaticFixed)
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
				IResource* res = image_components[i]->get_resource();
				hdk::assert(res->get_type() == ResourceType::Image, "Expected ResourceType of Texture, but is not a texture. Please submit a bug report.");
				if(res->get_access() != ResourceAccess::StaticFixed)
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
					.source_access = {vk2::AccessFlag::None},
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

	void RendererVulkan::setup_render_commands()
	{
		HDK_PROFZONE("Vulkan Frontend - RendererVulkan Setup Render Commands", 0xFFAAAA00);
		hdk::assert(!this->pipeline.is_compute(), "Running render command recording path, but pipeline is a compute pipeline. Logic error, please submit a bug report.");

		this->command.set_rendering_commands([this](vk2::CommandBufferRecording& recording, std::size_t framebuffer_id)
		{
			hdk::assert(framebuffer_id < this->output.get_output_framebuffers().size(), "Attempted to retrieve output framebuffer at index %zu, but there are only %zu framebuffers available. Please submit a bug report.", framebuffer_id, this->output.get_output_framebuffers().size());
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
				tz::BasicList<const vk2::DescriptorSet*> sets;
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

				hdk::vec2ui offset{0u, 0u};
				hdk::vec2ui extent = this->output.get_output_dimensions();

				if(this->get_output() != nullptr)
				{
					if(this->get_output()->scissor != ScissorRegion::null())
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

				const IComponent* idx_buf = this->get_component(this->state.graphics.index_buffer);
				const IComponent* ind_buf_gen = this->get_component(this->state.graphics.draw_buffer);;
				const auto* ind_buf = static_cast<const BufferComponentVulkan*>(ind_buf_gen);
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
						recording.draw_indirect
						({
							.draw_indirect_buffer = &ind_buf->vk_get_buffer(),
							.draw_count = static_cast<std::uint32_t>(ind_buf->get_resource()->data().size_bytes() / sizeof(VkDrawIndirectCommand)),
							.stride = static_cast<std::uint32_t>(sizeof(VkDrawIndirectCommand)),
							.offset = static_cast<VkDeviceSize>(0)
						});
					}
				}
				else
				{
					recording.bind_index_buffer
					({
						.index_buffer = &static_cast<const BufferComponentVulkan*>(idx_buf)->vk_get_buffer()
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
			recording.debug_end_label({});
		});
	}
	
	void RendererVulkan::setup_compute_commands()
	{
		HDK_PROFZONE("Vulkan Frontend - RendererVulkan Setup Compute Commands", 0xFFAAAA00);
		hdk::assert(this->pipeline.is_compute(), "Running compute command recording path, but pipeline is a graphics pipeline. Logic error, please submit a bug report.");

		this->command.set_rendering_commands([this](vk2::CommandBufferRecording& recording, std::size_t framebuffer_id)
		{
			hdk::assert(framebuffer_id < this->output.get_output_framebuffers().size(), "Attempted to retrieve output framebuffer at index %zu, but there are only %zu framebuffers available. Please submit a bug report.", framebuffer_id, this->output.get_output_framebuffers().size());
			recording.debug_begin_label
			({
				.name = this->debug_name
			});
			recording.bind_pipeline
			({
				.pipeline = &this->pipeline.get_pipeline(),
			});
			tz::BasicList<const vk2::DescriptorSet*> sets;
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

	void RendererVulkan::setup_work_commands()
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

	void RendererVulkan::handle_resize(const RendererResizeInfoVulkan& resize_info)
	{
		HDK_PROFZONE("Vulkan Frontend - RendererVulkan Handle Resize", 0xFFAAAA00);
		// Context: The top-level gl::Device has just been told by the window that it has been resized, and has recreated a new swapchain. Our old pointer to the swapchain `maybe_swapchain` correctly points to the new swapchain already, so we just have to recreate all the new state.
		this->command.wait_pending_commands_complete();
		this->output.create_output_resources(resize_info.new_output_images, resize_info.new_depth_image);
		this->pipeline.recreate(this->output.get_render_pass(), resize_info.new_dimensions, this->pipeline.is_wireframe_mode());
		this->setup_work_commands();
	}
}

#endif // TZ_VULKAN
