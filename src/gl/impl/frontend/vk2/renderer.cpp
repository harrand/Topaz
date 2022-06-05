#if TZ_VULKAN
#include "gl/impl/backend/vk2/tz_vulkan.hpp"
#include "core/profiling/zone.hpp"
#include "core/report.hpp"
#include "gl/declare/image_format.hpp"
#include "gl/impl/backend/vk2/fixed_function.hpp"
#include "gl/impl/backend/vk2/gpu_mem.hpp"
#include "gl/impl/backend/vk2/descriptors.hpp"
#include "gl/impl/backend/vk2/image_view.hpp"
#include "gl/impl/frontend/vk2/renderer.hpp"
#include "gl/impl/frontend/vk2/component.hpp"
#include "gl/impl/frontend/vk2/convert.hpp"
#include "gl/output.hpp"

namespace tz::gl
{
	using namespace tz::gl;

//--------------------------------------------------------------------------------------------------
	ResourceStorage::ResourceStorage(const RendererInfoVulkan& info, const vk2::LogicalDevice& ldev, std::size_t frame_in_flight_count):
	AssetStorageCommon<IResource>(info.get_resources()),
	components(),
	image_component_views(),
	basic_sampler(vk2::SamplerInfo
	{
		.device = &ldev,
		.min_filter = vk2::LookupFilter::Nearest,
		.mag_filter = vk2::LookupFilter::Nearest,
		.mipmap_mode = vk2::MipLookupFilter::Nearest,
		.address_mode_u = vk2::SamplerAddressMode::ClampToEdge,
		.address_mode_v = vk2::SamplerAddressMode::ClampToEdge,
		.address_mode_w = vk2::SamplerAddressMode::ClampToEdge,
	}),
	descriptor_layout(vk2::DescriptorLayout::null()),
	descriptor_pool(vk2::DescriptorPool::null()),
	descriptors(),
	frame_in_flight_count(frame_in_flight_count)
	{
		auto resources = info.get_resources();
		TZ_PROFZONE("Vulkan Frontend - RendererVulkan ResourceStorage Create", TZ_PROFCOL_YELLOW);
		std::vector<bool> buffer_id_to_variable_access;
		std::vector<bool> buffer_id_to_descriptor_visibility;
		std::size_t encountered_reference_count = 0;

		auto retrieve_resource_metadata = [this, &buffer_id_to_variable_access, &buffer_id_to_descriptor_visibility](IComponent* cmp)
		{
			IResource* res = cmp->get_resource();
			switch(res->get_type())
			{
				case ResourceType::Buffer:
				{
					auto buf = static_cast<BufferComponentVulkan*>(cmp);
					// We need to know this when creating the descriptors.
					buffer_id_to_variable_access.push_back(res->get_access() == ResourceAccess::DynamicVariable);
					buffer_id_to_descriptor_visibility.push_back(buf->vk_is_descriptor_relevant());
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
					auto* img = static_cast<ImageComponentVulkan*>(cmp);
					// We will need to create an image view. Let's get that out-of-the-way-now.
					vk2::Image& underlying_image = img->vk_get_image();
					this->image_component_views.emplace_back
						(vk2::ImageViewInfo{
							.image = &underlying_image,
							.aspect = vk2::ImageAspect::Colour
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
					tz_error("TODO: better error message lmao");
				break;
			}
		};

		for(std::size_t i = 0; i < this->count(); i++)
		{
			IResource* res = this->get(static_cast<tz::HandleValue>(i));
			IComponent* comp = nullptr;
			if(res == nullptr)
			{
				// If we see a null resource, it means we're looking for a component (resource reference).
				comp = const_cast<IComponent*>(info.get_components()[encountered_reference_count]);
				this->components.push_back(comp);
				encountered_reference_count++;

				res = comp->get_resource();
				// Also we'll write into the asset storage so it doesn't still think the resource is null.
				this->set(static_cast<tz::HandleValue>(i), res);
			}
			else
			{
				switch(res->get_type())
				{
					case ResourceType::Buffer:
					{
						this->components.push_back(tz::make_owned<BufferComponentVulkan>(*res, ldev));
					}
					break;
					case ResourceType::Image:
						this->components.push_back(tz::make_owned<ImageComponentVulkan>(*res, ldev));
					break;
					default:
						tz_error("Unrecognised ResourceType. Please submit a bug report.");
					break;
				}
				comp = this->components.back().get();
			}
			retrieve_resource_metadata(comp);
		}

		std::size_t buffer_count = this->resource_count_of(ResourceType::Buffer);
		std::size_t descriptor_buffer_count = std::count(buffer_id_to_descriptor_visibility.begin(), buffer_id_to_descriptor_visibility.end(), true);
		// So buffer_id_to_variable_access stores a bool for each buffer (in-order). True if the access is variable, false otherwise. However, if any buffer at all is variable, we will unfortunately try to write to them all in sync_descriptors.
		// Until we fix sync_descriptors to be a little less cavalier, we must add the descriptor-indexing flags and take the perf loss if there are *any* variable-sized buffer resources at all.
		{
			vk2::DescriptorLayoutBuilder lbuilder;
			lbuilder.set_device(ldev);
			// Each buffer gets their own binding id.
			for(std::size_t i = 0; i < buffer_count; i++)
			{
				// If the buffer shouldn't be visible to descriptors (e.g it is an index buffer) then skip it.
				if(!buffer_id_to_descriptor_visibility[i])
				{
					continue;
				}
				// TODO: Only add the necessary flags to the buffers with variable access instead of all of them if any have it. Dependent on changes to sync_descriptors.
				vk2::DescriptorFlags desc_flags;
				if(std::any_of(buffer_id_to_variable_access.begin(), buffer_id_to_variable_access.end(),
				[](bool b){return b;}))
				{
					desc_flags = 
					{
						vk2::DescriptorFlag::UpdateAfterBind,
						vk2::DescriptorFlag::UpdateUnusedWhilePending
					};
				}

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

		// If we have no resources at all, then we completely skip creating pools and sets.
		if(this->count() == 0)
		{
			return;
		}
		// Create pool limits. Enough for all of our resources. However, if we don't have any of a specific resource, we shouldn't add a limit at all for it (zero-size limits are not allowed).
		decltype(std::declval<vk2::DescriptorPoolInfo::PoolLimits>().limits) limits;
		if(buffer_count > 0)
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
		tz_assert(this->descriptors.success(), "Descriptor Pool allocation failed. Please submit a bug report.");
		this->sync_descriptors(true);
	}

	ResourceStorage::ResourceStorage(ResourceStorage&& move):
	AssetStorageCommon<IResource>(static_cast<AssetStorageCommon<IResource>&&>(move)),
	components(std::move(move.components)),
	image_component_views(std::move(move.image_component_views)),
	basic_sampler(std::move(move.basic_sampler)),
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
		std::swap(this->components, rhs.components);
		std::swap(this->image_component_views, rhs.image_component_views);
		std::swap(this->basic_sampler, rhs.basic_sampler);
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
		return this->components[static_cast<std::size_t>(static_cast<tz::HandleValue>(handle))].get();
	}

	IComponent* ResourceStorage::get_component(ResourceHandle handle)
	{
		return this->components[static_cast<std::size_t>(static_cast<tz::HandleValue>(handle))].get();
	}

	const vk2::DescriptorLayout& ResourceStorage::get_descriptor_layout() const
	{
		return this->descriptor_layout;
	}

	std::span<const vk2::DescriptorSet> ResourceStorage::get_descriptor_sets() const
	{
		return this->descriptors.sets;
	}

	const IComponent* ResourceStorage::try_get_index_buffer() const
	{
		auto iter = std::find_if(this->components.begin(), this->components.end(),
		[](const auto& component_ptr)
		{
			auto res = component_ptr->get_resource();
			return res->get_type() == ResourceType::Buffer && res->get_flags().contains(ResourceFlag::IndexBuffer);
		});
		if(iter == this->components.end())
		{
			return nullptr;
		}
		return iter->get();
	}

	std::size_t ResourceStorage::resource_count_of(ResourceType type) const
	{
		return std::count_if(this->components.begin(), this->components.end(),
		[type](const auto& component_ptr)
		{
			return component_ptr->get_resource()->get_type() == type;
		});
	}

	void ResourceStorage::notify_image_recreated(tz::gl::ResourceHandle image_resource_handle)
	{
		// ImageComponent's underlying vk2::Image was recently replaced with another. This means this->image_component_views[id corresponding to handle] is wrong and needs to be remade.
		std::size_t img_view_idx = 0;
		auto handle_val = static_cast<std::size_t>(static_cast<tz::HandleValue>(image_resource_handle));
		for(std::size_t i = 0; i < handle_val; i++)
		{
			if(this->get(static_cast<tz::HandleValue>(i))->get_type() == ResourceType::Image)
			{
				img_view_idx++;
			}
		}
		this->image_component_views[img_view_idx] =
		{{
			.image = &(static_cast<ImageComponentVulkan*>(this->get_component(image_resource_handle))->vk_get_image()),
			.aspect = vk2::ImageAspect::Colour
		}};
	}

	void ResourceStorage::sync_descriptors(bool write_everything)
	{
		TZ_PROFZONE("Vulkan Frontend - RendererVulkan ResourceStorage Descriptor Sync", TZ_PROFCOL_YELLOW);
		std::vector<BufferComponentVulkan*> buffers;
		for(auto& component_ptr : this->components)
		{
			if(component_ptr->get_resource()->get_type() == ResourceType::Buffer)
			{
				buffers.push_back(component_ptr.as<BufferComponentVulkan>());
			}
		}
		std::size_t descriptor_buffer_count = std::count_if(buffers.begin(), buffers.end(), [](BufferComponentVulkan* buf)
		{
			return buf->vk_is_descriptor_relevant();
		});

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
				if(!buffers[j]->vk_is_descriptor_relevant())
				{
					continue;
				}
				set_edit.set_buffer(j,
				{
					.buffer = &buffers[j]->vk_get_buffer(),
					.buffer_offset = 0,
					.buffer_write_size = buffers[j]->vk_get_buffer().size()
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
						.sampler = &this->basic_sampler,
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
				tz_assert(row_pitch > resource_row_pitch, "Linear CPU vk2::Image row data is negatively-padded? Assuming ImageResource data is tightly-packed, it uses %zu bytes per row, but the actual image uses %zu? It should be using more, not less!", resource_row_pitch, row_pitch);
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

	OutputManager::OutputManager(const IOutput* output, std::span<vk2::Image> window_buffer_images, bool create_depth_images, const vk2::LogicalDevice& ldev):
	output(output != nullptr ? output->unique_clone() : nullptr),
	ldev(&ldev),
	window_buffer_images(window_buffer_images),
	window_buffer_depth_images(),
	output_imageviews(),
	render_pass(vk2::RenderPass::null()),
	output_framebuffers()
	{
		TZ_PROFZONE("Vulkan Frontend - RendererVulkan OutputManager Create", TZ_PROFCOL_YELLOW);
		this->create_output_resources(this->window_buffer_images, create_depth_images);
	}

	OutputManager::OutputManager(OutputManager&& move):
	output(std::move(move.output)),
	ldev(move.ldev),
	window_buffer_images(std::move(move.window_buffer_images)),
	window_buffer_depth_images(std::move(move.window_buffer_depth_images)),
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
		std::swap(this->window_buffer_images, rhs.window_buffer_images);
		std::swap(this->window_buffer_depth_images, rhs.window_buffer_depth_images);
		std::swap(this->output_imageviews, rhs.output_imageviews);
		std::swap(this->output_depth_imageviews, rhs.output_depth_imageviews);
		std::swap(this->render_pass, rhs.render_pass);
		std::swap(this->output_framebuffers, rhs.output_framebuffers);
		for(auto& fb : this->output_framebuffers)
		{
			fb.set_render_pass(this->render_pass);
		}
		return *this;
	}

	const vk2::RenderPass& OutputManager::get_render_pass() const
	{
		return this->render_pass;
	}

	std::vector<OutputImageState> OutputManager::get_output_images()
	{
		if(this->output == nullptr || this->output->get_target() == OutputTarget::Window)
		{
			// We're rendering into a window (which may be headless). The Device contained the swapchain images (or the offline headless images). Simply return those.
			std::vector<OutputImageState> ret(this->window_buffer_images.size());
			for(std::size_t i = 0; i < this->window_buffer_images.size(); i++)
			{
				ret[i] =
				{
					.colour_attachments = {&this->window_buffer_images[i]},
					.depth_attachment = &this->window_buffer_depth_images[i]
				};
			}
			return ret;
		}
		else if(this->output->get_target() == OutputTarget::OffscreenImage)
		{
			// We have been provided an ImageOutput which will contain an ImageComponentVulkan. We need to retrieve that image and return a span covering it.
			// TODO: Support multiple-render-targets.
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

			std::vector<OutputImageState> ret(this->window_buffer_images.size(), out_image);
			tz_assert(!out.has_depth_attachment(), "Depth attachment on an ImageOutput is not yet implemented");
			return ret;
		}
		else
		{
			tz_error("Unrecognised OutputTarget. Please submit a bug report.");
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

	tz::Vec2ui OutputManager::get_output_dimensions() const
	{
		tz_assert(!this->output_imageviews.empty(), "OutputManager had no output views, so impossible to retrieve viewport dimensions. Please submit a bug report.");
		return this->output_imageviews.front().colour_views.front().get_image().get_dimensions();
	}
	
	bool OutputManager::has_depth_images() const
	{
		return !this->window_buffer_depth_images.empty();
	}

	void OutputManager::create_output_resources(std::span<vk2::Image> window_buffer_images, bool create_depth_images)
	{
		TZ_PROFZONE("Vulkan Frontend - RendererVulkan OutputManager (Output Resources Creation)", TZ_PROFCOL_YELLOW);
		this->window_buffer_images = window_buffer_images;
		this->window_buffer_depth_images.clear();
		this->output_imageviews.clear();
		this->output_depth_imageviews.clear();
		this->output_framebuffers.clear();

		//tz_assert(!this->get_output_images().empty(), "RendererVulkan OutputManager was not given any output images. Please submit a bug report.");
		this->window_buffer_depth_images.reserve(this->window_buffer_images.size());
		this->output_depth_imageviews.reserve(this->window_buffer_images.size());
		for(const vk2::Image& window_buffer_image : this->window_buffer_images)
		{
			// If we need depth images for depth testing, we'll create them based off of the existing output images we are provided from the Device.
			if(create_depth_images)
			{
				// If we're rendering to a TextureOutput and it has a depth attachment, then we want to use that. Otherwise, we create one now.
				if(this->output != nullptr && this->output->get_target() == OutputTarget::OffscreenImage && static_cast<const ImageOutput*>(this->output.get())->has_depth_attachment())
				{
					tz_error("Sorry, depth attachment to image outputs are not yet implemented.");
					this->window_buffer_depth_images.push_back(vk2::Image::null());
				}
				else
				{
					this->window_buffer_depth_images.push_back
					(vk2::ImageInfo{
						.device = this->ldev,
						.format = vk2::ImageFormat::Depth32_SFloat,
						.dimensions = window_buffer_image.get_dimensions(),
						.usage = {vk2::ImageUsage::DepthStencilAttachment},
						.residency = vk2::MemoryResidency::GPU
					});
				}
			}
			else
			{
				// Otherwise, just create empty ones and don't bother using them.
				this->window_buffer_depth_images.push_back(vk2::Image::null());
			}
		}
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
					.aspect = vk2::ImageAspect::Colour
				}});
			}
			this->output_imageviews.push_back(std::move(out_view));

			// If we made depth images earlier, create views for them too.
			if(create_depth_images)
			{
				this->output_depth_imageviews.push_back
				(vk2::ImageViewInfo{
					.image = &this->window_buffer_depth_images[i],
					.aspect = vk2::ImageAspect::Depth
				});
			}
			else
			{
				this->output_depth_imageviews.push_back(vk2::ImageView::null());
			}
		}

		for(const OutputImageViewState& out_view : this->output_imageviews)
		{
			tz_assert(std::equal(out_view.colour_views.begin(), out_view.colour_views.end(), out_view.colour_views.begin(), [](const vk2::ImageView& a, const vk2::ImageView& b){return a.get_image().get_format() == b.get_image().get_format();}), "Detected that not every output image in a RendererVulkan has the same format. This is not permitted as RenderPasses would not be compatible. Please submit a bug report.");
		}

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
			tz_error("Unknown RendererOutputType. Please submit a bug report.");
		}
		
		// Our renderpass is no longer so simple with the possibilty of multiple colour outputs.
		std::uint32_t colour_output_length = output_image_copy.front().colour_attachments.length();

		vk2::RenderPassBuilder rbuilder;
		rbuilder.set_device(*this->ldev);
		for(vk2::Image* colour_image : output_image_copy.front().colour_attachments)
		{
			rbuilder.with_attachment
			({
				.format = colour_image->get_format(),
				.colour_depth_store = vk2::StoreOp::Store,
				.initial_layout = vk2::ImageLayout::Undefined,
				.final_layout = final_layout
			});
		}
		if(create_depth_images)
		{
			rbuilder.with_attachment
			({
				.format = this->window_buffer_depth_images.front().get_format(),
				.colour_depth_store = vk2::StoreOp::DontCare,
				.initial_layout = vk2::ImageLayout::Undefined,
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
		if(create_depth_images)
		{
			sbuilder.with_depth_stencil_attachment
			({
				.attachment_idx = colour_output_length,
				.current_layout = vk2::ImageLayout::DepthStencilAttachment
			});
		}

		this->render_pass = rbuilder.with_subpass(sbuilder.build()).build();

		for(std::size_t i = 0; i < this->output_imageviews.size(); i++)
		{
			tz::Vec2ui dims = this->output_imageviews[i].colour_views.front().get_image().get_dimensions();
			tz::BasicList<vk2::ImageView*> attachments;
			attachments.resize(this->output_imageviews[i].colour_views.length());
			std::transform(this->output_imageviews[i].colour_views.begin(), this->output_imageviews[i].colour_views.end(), attachments.begin(),
			[](vk2::ImageView& colour_view)
			{
				return &colour_view;
			});
			if(create_depth_images)
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

	GraphicsPipelineManager::GraphicsPipelineManager
	(
		const ShaderInfo& sinfo,
		const vk2::DescriptorLayout& dlayout,
		const vk2::RenderPass& render_pass,
		std::size_t frame_in_flight_count,
		tz::Vec2ui viewport_dimensions,
		bool depth_testing_enabled,
		bool alpha_blending_enabled
	):
	shader(this->make_shader(dlayout.get_device(), sinfo)),
	pipeline_layout(this->make_pipeline_layout(dlayout, frame_in_flight_count)),
	graphics_pipeline(this->make_pipeline(viewport_dimensions, depth_testing_enabled, alpha_blending_enabled, render_pass)),
	depth_testing_enabled(depth_testing_enabled)
	{
		// TODO: Implement vk2::LogicalDevice equality operator
		//tz_assert(dlayout.get_device() == render_pass.get_device(), "");
	}

	GraphicsPipelineManager::GraphicsPipelineManager(GraphicsPipelineManager&& move):
	shader(std::move(move.shader)),
	pipeline_layout(std::move(move.pipeline_layout)),
	graphics_pipeline(std::move(move.graphics_pipeline)),
	depth_testing_enabled(move.depth_testing_enabled)
	{
		this->graphics_pipeline.set_layout(this->pipeline_layout);
	}

	GraphicsPipelineManager& GraphicsPipelineManager::operator=(GraphicsPipelineManager&& rhs)
	{
		std::swap(this->shader, rhs.shader);
		std::swap(this->pipeline_layout, rhs.pipeline_layout);
		std::swap(this->graphics_pipeline, rhs.graphics_pipeline);
		std::swap(this->depth_testing_enabled, rhs.depth_testing_enabled);
		this->graphics_pipeline.set_layout(this->pipeline_layout);

		return *this;
	}

	const vk2::Pipeline& GraphicsPipelineManager::get_pipeline() const
	{
		return this->graphics_pipeline;
	}

	void GraphicsPipelineManager::recreate(const vk2::RenderPass& new_render_pass, tz::Vec2ui new_viewport_dimensions, bool wireframe_mode)
	{
		this->wireframe_mode = wireframe_mode;
		TZ_PROFZONE("Vulkan Frontend - RendererVulkan GraphicsPipelineManager Recreate", TZ_PROFCOL_YELLOW);
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
					.viewport = vk2::create_basic_viewport(static_cast<tz::Vec2>(new_viewport_dimensions)),
					.rasteriser = {.polygon_mode = (this->wireframe_mode ? vk2::PolygonMode::Line : vk2::PolygonMode::Fill)},
					.depth_stencil =
					{
						.depth_testing = this->depth_testing_enabled,
						.depth_writes = this->depth_testing_enabled
					},
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
		TZ_PROFZONE("Vulkan Frontend - RendererVulkan GraphicsPipelineManager (Shader Create)", TZ_PROFCOL_YELLOW);
		std::vector<char> vtx_src, frg_src, cmp_src;
		tz::BasicList<vk2::ShaderModuleInfo> modules;
		if(sinfo.has_shader(ShaderStage::Compute))
		{
			tz_assert(!sinfo.has_shader(ShaderStage::Vertex), "Shader has compute shader and vertex shader. These are mutually exclusive.");
			tz_assert(!sinfo.has_shader(ShaderStage::Fragment), "Shader has compute shader and fragment shader. These are mutually exclusive.");
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
			tz_assert(sinfo.has_shader(ShaderStage::Vertex), "ShaderInfo must contain a non-empty vertex shader if no compute shader is present.");
			tz_assert(sinfo.has_shader(ShaderStage::Fragment), "ShaderInfo must contain a non-empty fragment shader if no compute shader is present.");
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

	vk2::GraphicsPipelineInfo GraphicsPipelineManager::make_graphics_pipeline(tz::Vec2ui viewport_dimensions, bool depth_testing_enabled, bool alpha_blending_enabled, const vk2::RenderPass& render_pass) const
	{
		tz::BasicList<vk2::ColourBlendState::AttachmentState> alpha_blending_options;
		alpha_blending_options.resize(render_pass.get_info().total_colour_attachment_count());
		std::fill(alpha_blending_options.begin(), alpha_blending_options.end(), alpha_blending_enabled ? vk2::ColourBlendState::alpha_blending() : vk2::ColourBlendState::no_blending());
		return
		{
			.shaders = this->shader.native_data(),
			.state =
			{
				.viewport = vk2::create_basic_viewport(static_cast<tz::Vec2>(viewport_dimensions)),
				.depth_stencil =
				{
					.depth_testing = depth_testing_enabled,
					.depth_writes = depth_testing_enabled
				},
				.colour_blend =
				{
					.attachment_states = alpha_blending_options,
					.logical_operator = VK_LOGIC_OP_COPY
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

	vk2::Pipeline GraphicsPipelineManager::make_pipeline(tz::Vec2ui viewport_dimensions, bool depth_testing_enabled, bool alpha_blending_enabled, const vk2::RenderPass& render_pass) const
	{
		if(this->is_compute())
		{
			return {this->make_compute_pipeline()};
		}

		return {this->make_graphics_pipeline(viewport_dimensions, depth_testing_enabled, alpha_blending_enabled, render_pass)};
	}

//--------------------------------------------------------------------------------------------------

	CommandProcessor::CommandProcessor(vk2::LogicalDevice& ldev, std::size_t frame_in_flight_count, OutputTarget output_target, std::span<vk2::Framebuffer> output_framebuffers, bool instant_compute_enabled):
	requires_present(output_target == OutputTarget::Window),
	instant_compute_enabled(instant_compute_enabled),
	graphics_queue(ldev.get_hardware_queue
	({
		.field = {vk2::QueueFamilyType::Graphics},
		.present_support = this->requires_present
	})),
	compute_queue(ldev.get_hardware_queue
	({
		.field = {vk2::QueueFamilyType::Compute},
		.present_support = false
	})),
	command_pool
	({
		.queue = this->graphics_queue,
		.flags = {vk2::CommandPoolFlag::Reusable}
	}),
	commands(this->command_pool.allocate_buffers
	({
		.buffer_count = static_cast<std::uint32_t>(frame_in_flight_count + 1)
	})),
	frame_in_flight_count(frame_in_flight_count),
	image_semaphores(),
	render_work_semaphores(),
	in_flight_fences(),
	images_in_flight(this->frame_in_flight_count, nullptr)
	{
		tz_assert(this->graphics_queue != nullptr, "Could not retrieve graphics present queue. Either your machine does not meet requirements, or (more likely) a logical error. Please submit a bug report.");
		tz_assert(this->compute_queue != nullptr, "Could not retrieve compute queue. Either your machine does not meet requirements, or (more likely) a logical error. Please submit a bug report.");
		tz_assert(output_framebuffers.size() == this->frame_in_flight_count, "Provided incorrect number of output framebuffers. We must have enough framebuffers for each frame we have in flight. Provided %zu framebuffers, but need %zu because that's how many frames we have in flight.", output_framebuffers.size(), this->frame_in_flight_count);
		tz_assert(this->commands.success(), "Failed to allocate from CommandPool");
		for(std::size_t i = 0; i < this->frame_in_flight_count; i++)
		{
			this->image_semaphores.emplace_back(ldev);
			this->render_work_semaphores.emplace_back(ldev);
			this->in_flight_fences.emplace_back
			(vk2::FenceInfo{
				.device = &ldev,
				.initially_signalled = true
			});
		}
	}

	std::span<const vk2::CommandBuffer> CommandProcessor::get_render_command_buffers() const
	{
		return {this->commands.buffers.begin(), this->frame_in_flight_count};
	}

	std::span<vk2::CommandBuffer> CommandProcessor::get_render_command_buffers()
	{
		return {this->commands.buffers.begin(), this->frame_in_flight_count};
	}

	CommandProcessor::RenderWorkSubmitResult CommandProcessor::do_render_work(vk2::Swapchain* maybe_swapchain)
	{
		TZ_PROFZONE("Vulkan Frontend - RendererVulkan CommandProcessor (Do Render Work)", TZ_PROFCOL_YELLOW);
		tz_assert(maybe_swapchain != nullptr, "Trying to do render work with presentation, but no Swapchain provided. Please submit a bug report.");
		vk2::Swapchain& swapchain = *maybe_swapchain;
		// Submit & Present
		this->in_flight_fences[this->current_frame].wait_until_signalled();
		if(requires_present)
		{
			this->output_image_index = swapchain.acquire_image
			({
				.signal_semaphore = &this->image_semaphores[current_frame]
			}).image_index;

			const vk2::Fence*& target_image = this->images_in_flight[this->output_image_index];
			if(target_image != nullptr)
			{
				target_image->wait_until_signalled();
			}
			target_image = &this->in_flight_fences[this->output_image_index];
		}
		else
		{
			this->output_image_index = this->current_frame;
		}

		this->in_flight_fences[this->current_frame].unsignal();
		tz::BasicList<vk2::hardware::Queue::SubmitInfo::WaitInfo> waits;
		if(requires_present)
		{
			waits =
			{
				vk2::hardware::Queue::SubmitInfo::WaitInfo
				{
					.wait_semaphore = &this->image_semaphores[this->current_frame],
					.wait_stage = vk2::PipelineStage::ColourAttachmentOutput
				}
			};
		}
		tz::BasicList<const vk2::BinarySemaphore*> sem_signals;
		if(requires_present)
		{
			sem_signals = {&this->render_work_semaphores[this->current_frame]};
		}
		this->graphics_queue->submit
		({
			.command_buffers = {&this->get_render_command_buffers()[this->output_image_index]},
			.waits = waits,
			.signal_semaphores = sem_signals,
			.execution_complete_fence = &this->in_flight_fences[this->current_frame]
		});

		CommandProcessor::RenderWorkSubmitResult result;

		if(requires_present)
		{
			result.present = this->graphics_queue->present
			({
				.wait_semaphores = {&this->render_work_semaphores[this->current_frame]},
				.swapchain = maybe_swapchain,
				.swapchain_image_index = this->output_image_index
			});
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
		TZ_PROFZONE("Vulkan Frontend - RendererVulkan CommandProcessor (Do Compute Work)", TZ_PROFCOL_YELLOW);

		if(!this->instant_compute_enabled)
		{
			this->in_flight_fences[this->current_frame].wait_until_signalled();
		}
		this->in_flight_fences[this->current_frame].unsignal();
		this->compute_queue->submit
		({
			.command_buffers = {&this->get_render_command_buffers().front()},
			.waits = {},
			.signal_semaphores = {},
			.execution_complete_fence = &this->in_flight_fences[this->current_frame]
		});

		if(this->instant_compute_enabled)
		{
			this->in_flight_fences[this->current_frame].wait_until_signalled();
		}
	}

	void CommandProcessor::wait_pending_commands_complete()
	{
		TZ_PROFZONE("Vulkan Frontend - RendererVulkan CommandProcessor (Waiting on commands to complete)", TZ_PROFCOL_YELLOW);
		for(const vk2::Fence& fence : this->in_flight_fences)
		{
			fence.wait_until_signalled();
		}
	}

//--------------------------------------------------------------------------------------------------

	RendererVulkan::RendererVulkan(const RendererInfoVulkan& info, const RendererDeviceInfoVulkan& device_info):
	ldev(device_info.device),
	resources(info, *this->ldev, this->get_frame_in_flight_count(device_info)),
	output(info.get_output(), device_info.output_images, !info.get_options().contains(RendererOption::NoDepthTesting), *this->ldev),
	pipeline(info.shader(), this->resources.get_descriptor_layout(), this->output.get_render_pass(), this->get_frame_in_flight_count(device_info), output.get_output_dimensions(), !info.get_options().contains(RendererOption::NoDepthTesting), info.get_options().contains(RendererOption::AlphaBlending)),
	command(*this->ldev, this->get_frame_in_flight_count(device_info), info.get_output() != nullptr ? info.get_output()->get_target() : OutputTarget::Window, this->output.get_output_framebuffers(), info.get_options().contains(RendererOption::BlockingCompute)),
	maybe_swapchain(device_info.maybe_swapchain),
	options(info.get_options()),
	clear_colour(info.get_clear_colour()),
	compute_kernel(info.get_compute_kernel())
	{
		TZ_PROFZONE("Vulkan Frontend - RendererVulkan Create", TZ_PROFCOL_YELLOW);
		// If we're not headless, we should register a callback for our lifetime.
		if(info.get_output() == nullptr || info.get_output()->get_target() == OutputTarget::Window)
		{
			this->device_resize_callback = device_info.resize_callback;
			this->window_resize_callback = this->device_resize_callback->add_callback([this](RendererResizeInfoVulkan resize_info){this->handle_resize(resize_info);});
		}

		this->setup_static_resources();
		this->setup_work_commands();
	}

	RendererVulkan::RendererVulkan(RendererVulkan&& move):
	ldev(move.ldev),
	resources(std::move(move.resources)),
	output(std::move(move.output)),
	pipeline(std::move(move.pipeline)),
	command(std::move(move.command)),
	maybe_swapchain(move.maybe_swapchain),
	options(move.options),
	clear_colour(move.clear_colour),
	tri_count(move.tri_count),
	device_resize_callback(move.device_resize_callback),
	window_resize_callback(move.window_resize_callback)
	{
		this->device_resize_callback->remove_callback(move.window_resize_callback);
		this->device_resize_callback->remove_callback(this->window_resize_callback);
		this->window_resize_callback = this->device_resize_callback->add_callback([this](RendererResizeInfoVulkan resize_info){this->handle_resize(resize_info);});
	}

	RendererVulkan::~RendererVulkan()
	{
		if(this->device_resize_callback != nullptr)
		{
			this->device_resize_callback->remove_callback(this->window_resize_callback);
			this->window_resize_callback = tz::nullhand;
		}
		this->ldev->wait_until_idle();
	}

	RendererVulkan& RendererVulkan::operator=(RendererVulkan&& rhs)
	{
		std::swap(this->ldev, rhs.ldev);
		std::swap(this->resources, rhs.resources);
		std::swap(this->output, rhs.output);
		std::swap(this->pipeline, rhs.pipeline);
		std::swap(this->command, rhs.command);
		std::swap(this->maybe_swapchain, rhs.maybe_swapchain);
		std::swap(this->options, rhs.options);
		std::swap(this->clear_colour, rhs.clear_colour);
		std::swap(this->tri_count, rhs.tri_count);
		std::swap(this->device_resize_callback, rhs.device_resize_callback);
		std::swap(this->window_resize_callback, rhs.window_resize_callback);
		this->device_resize_callback->remove_callback(rhs.window_resize_callback);
		this->device_resize_callback->remove_callback(this->window_resize_callback);
		this->window_resize_callback = this->device_resize_callback->add_callback([this](RendererResizeInfoVulkan resize_info){this->handle_resize(resize_info);});
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

	const RendererOptions& RendererVulkan::get_options() const
	{
		return this->options;
	}

	void RendererVulkan::render()
	{
		TZ_PROFZONE("Vulkan Frontend - RendererVulkan Render", TZ_PROFCOL_YELLOW);
		this->resources.write_padded_image_data();
		if(this->pipeline.is_compute())
		{
			this->command.do_compute_work();
		}
		else
		{
			CommandProcessor::RenderWorkSubmitResult result = this->command.do_render_work(this->maybe_swapchain);
			switch(result.present)
			{
				case vk2::hardware::Queue::PresentResult::Success_Suboptimal:
				[[fallthrough]];
				case vk2::hardware::Queue::PresentResult::Success:

				break;
				default:
					tz_error("Presentation failed, but for unknown reason. Please submit a bug report.");
				break;
			}
		}
	}

	void RendererVulkan::render(unsigned int tri_count)
	{
		if(this->tri_count != tri_count)
		{
			this->tri_count = tri_count;
			this->command.wait_pending_commands_complete();
			this->setup_work_commands();
		}
		this->render();
	}

	void RendererVulkan::edit(const RendererEditRequest& edit_request)
	{
		TZ_PROFZONE("Vulkan Frontend - RendererVulkan Edit", TZ_PROFCOL_YELLOW);
		bool work_commands_need_recording = false;
		bool pipeline_needs_recreating = false;
		if(edit_request.compute_edit.has_value() && edit_request.compute_edit.value().kernel != this->compute_kernel)
		{
			this->compute_kernel = edit_request.compute_edit.value().kernel;
			work_commands_need_recording = true;
		}
		else if(edit_request.render_state_edit.has_value() && this->pipeline.is_wireframe_mode() != edit_request.render_state_edit.value().wireframe_mode)
		{
			pipeline_needs_recreating = true;
		}
		else if(edit_request.component_edits.empty())
		{
			return;
		}
		// We have buffer/image components we need to edit.
		// Firstly make sure all render work is done, then we need to update the resource storage and then write new descriptors for resized resources.
		this->command.wait_pending_commands_complete();
		// Now, if we resized any static resources we're going to have to run scratch commands again.
		bool resized_static_resources = false;
		for(const RendererComponentEditRequest& component_edit : edit_request.component_edits)
		{
			std::visit(
			[this, &work_commands_need_recording](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;
				if constexpr(std::is_same_v<T, RendererBufferComponentEditRequest>)
				{
					auto buf_comp = static_cast<BufferComponentVulkan*>(this->get_component(arg.buffer_handle));
					if(buf_comp->size() == arg.size)
					{
						return;
					}
					buf_comp->resize(arg.size);
					if(buf_comp == this->resources.try_get_index_buffer())
					{
						// Index buffer has resized, meaning there will be a new underlying buffer object. This means the rendering commands need to be recorded because the bind command for the index buffer now references the dead old buffer.
						work_commands_need_recording = true;
					}
					// Now update all descriptors.
					this->resources.sync_descriptors(false);
				}
				else if constexpr(std::is_same_v<T, RendererImageComponentEditRequest>)
				{
					auto img_comp = static_cast<ImageComponentVulkan*>(this->get_component(arg.image_handle));
					if(img_comp->get_dimensions() == arg.dimensions)
					{
						return;
					}
					img_comp->resize(arg.dimensions);
					// An imageview was looking at the old image, let's update that.
					this->resources.notify_image_recreated(arg.image_handle);
					// New image so descriptor array needs to be re-written to.
					this->resources.sync_descriptors(true);
				}
				else
				{
					tz_error("Unsupported Variant Type");
				}
			}, component_edit);
		}
		if(pipeline_needs_recreating)
		{
			this->pipeline.recreate(this->output.get_render_pass(), this->output.get_output_dimensions(), edit_request.render_state_edit.value().wireframe_mode);
		}
		if(work_commands_need_recording)
		{
			this->setup_work_commands();
		}
		if(resized_static_resources)
		{
			this->setup_static_resources();
		}
	}

	void RendererVulkan::setup_static_resources()
	{
		TZ_PROFZONE("Vulkan Frontend - RendererVulkan Setup Static Resources", TZ_PROFCOL_YELLOW);
		// Create staging buffers for each buffer and texture resource, and then fill the data with the resource data.
		std::vector<BufferComponentVulkan*> buffer_components;
		std::vector<ImageComponentVulkan*> image_components;
		for(std::size_t i = 0; i < this->resource_count(); i++)
		{
			IComponent* icomp = this->get_component(static_cast<tz::HandleValue>(i));
			switch(icomp->get_resource()->get_type())
			{
				case ResourceType::Buffer:
					buffer_components.push_back(static_cast<BufferComponentVulkan*>(icomp));
				break;
				case ResourceType::Image:
					image_components.push_back(static_cast<ImageComponentVulkan*>(icomp));
				break;
				default:
					tz_error("Unknown ResourceType.");
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
				.size_bytes = tz::gl::pixel_size_bytes(img->get_format()) * img->get_dimensions()[0] * img->get_dimensions()[1],
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
				tz_assert(res->get_type() == ResourceType::Buffer, "Expected ResourceType of buffer, but is not a buffer. Please submit a bug report.");
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
				tz_assert(res->get_type() == ResourceType::Image, "Expected ResourceType of Texture, but is not a texture. Please submit a bug report.");
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
					.image_aspects = {vk2::ImageAspectFlag::Colour}
				});
				recording.buffer_copy_image
				({
					.src = &staging_image_buffers[i],
					.dst = &image_components[i]->vk_get_image(),
					.image_aspects = {vk2::ImageAspectFlag::Colour}
				});
				recording.transition_image_layout
				({
					.image = &image_components[i]->vk_get_image(),
					.target_layout = vk2::ImageLayout::ShaderResource,
					.source_access = {vk2::AccessFlag::TransferOperationWrite},
					.destination_access = {vk2::AccessFlag::ShaderResourceRead},
					.source_stage = vk2::PipelineStage::TransferCommands,
					.destination_stage = vk2::PipelineStage::FragmentShader,
					.image_aspects = {vk2::ImageAspectFlag::Colour}
				});
			}
		});
	}

	void RendererVulkan::setup_render_commands()
	{
		TZ_PROFZONE("Vulkan Frontend - RendererVulkan Setup Render Commands", TZ_PROFCOL_YELLOW);
		tz_assert(!this->pipeline.is_compute(), "Running render command recording path, but pipeline is a compute pipeline. Logic error, please submit a bug report.");

		this->command.set_rendering_commands([this](vk2::CommandBufferRecording& recording, std::size_t framebuffer_id)
		{
			tz_assert(framebuffer_id < this->output.get_output_framebuffers().size(), "Attempted to retrieve output framebuffer at index %zu, but there are only %zu framebuffers available. Please submit a bug report.", framebuffer_id, this->output.get_output_framebuffers().size());

			vk2::CommandBufferRecording::RenderPassRun run{this->output.get_output_framebuffers()[framebuffer_id], recording, this->clear_colour};
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

			const IComponent* idx_buf = this->resources.try_get_index_buffer();
			if(idx_buf == nullptr)
			{
				recording.draw
				({
					.vertex_count = 3 * this->tri_count,
					.instance_count = 1,
					.first_vertex = 0,
					.first_instance = 0
				});
			}
			else
			{
				recording.bind_index_buffer
				({
					.index_buffer = &static_cast<const BufferComponentVulkan*>(idx_buf)->vk_get_buffer()
				});
				recording.draw_indexed
				({
					.index_count = 3 * this->tri_count
				});
			}
		});
	}
	
	void RendererVulkan::setup_compute_commands()
	{
		TZ_PROFZONE("Vulkan Frontend - RendererVulkan Setup Compute Commands", TZ_PROFCOL_YELLOW);
		tz_assert(this->pipeline.is_compute(), "Running compute command recording path, but pipeline is a graphics pipeline. Logic error, please submit a bug report.");

		this->command.set_rendering_commands([this](vk2::CommandBufferRecording& recording, std::size_t framebuffer_id)
		{
			tz_assert(framebuffer_id < this->output.get_output_framebuffers().size(), "Attempted to retrieve output framebuffer at index %zu, but there are only %zu framebuffers available. Please submit a bug report.", framebuffer_id, this->output.get_output_framebuffers().size());
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

			tz_assert(!this->resources.try_get_index_buffer(), "Compute Renderer has an index buffer applied. This doesn't make any sense. Please submit a bug report.");
			{
				recording.dispatch
				({
					.groups = this->compute_kernel
				});
			}
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
		TZ_PROFZONE("Vulkan Frontend - RendererVulkan Handle Resize", TZ_PROFCOL_YELLOW);
		// Context: The top-level gl::Device has just been told by the window that it has been resized, and has recreated a new swapchain. Our old pointer to the swapchain `maybe_swapchain` correctly points to the new swapchain already, so we just have to recreate all the new state.
		this->command.wait_pending_commands_complete();
		this->output.create_output_resources(resize_info.new_output_images, this->output.has_depth_images());
		this->pipeline.recreate(this->output.get_render_pass(), resize_info.new_dimensions, this->pipeline.is_wireframe_mode());
		this->setup_work_commands();
	}

	std::size_t RendererVulkan::get_frame_in_flight_count(const RendererDeviceInfoVulkan& device_info) const
	{
		std::uint32_t min = device_info.device->get_hardware().get_surface_capabilities().min_image_count;
		std::uint32_t max = device_info.device->get_hardware().get_surface_capabilities().max_image_count;
		return std::clamp<std::uint32_t>(3u, min, max);
	}
}

#endif // TZ_VULKAN
