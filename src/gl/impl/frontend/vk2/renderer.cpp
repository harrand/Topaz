#include "gl/impl/backend/vk2/logical_device.hpp"
#include "gl/impl/backend/vk2/pipeline_layout.hpp"
#include "gl/impl/backend/vk2/sampler.hpp"
#if TZ_VULKAN
#include "gl/declare/renderer.hpp"
#include "gl/resource.hpp"
#include "gl/impl/backend/vk2/image_view.hpp"
#include "gl/impl/frontend/vk2/renderer.hpp"
#include "gl/impl/frontend/vk2/convert.hpp"
#include "gl/impl/frontend/vk2/shader.hpp"

namespace tz::gl
{
	namespace detail
	{
		IOManager::IOManager(IOManagerInfo info):
		output_views(),
		render_pass(vk2::RenderPass::null())
		{
			tz_assert(!info.output_images.empty(), "RendererVulkan2 IOManager was not given any output images. Please submit a bug report.");
			for(vk2::Image& output_image : info.output_images)
			{
				this->output_views.push_back
				(vk2::ImageViewInfo{
					.image = &output_image,
					.aspect = vk2::ImageAspect::Colour
				});
			}

			tz_assert(std::equal(this->output_views.begin(), this->output_views.end(), this->output_views.begin(), [](const vk2::ImageView& a, const vk2::ImageView& b){return a.get_image().get_format() == b.get_image().get_format();}), "Detected that not every output image in a RendererVulkan2 has the same format. This is not submitted as RenderPasses would not be compatible. Please submit a bug report.");

			// Create a super basic RenderPass.
			{
				// We're matching the ImageFormat of the provided output image.
				vk2::ImageLayout final_layout;
				switch(info.output_type)
				{
					case RendererOutputType::Texture:
						final_layout = vk2::ImageLayout::ColourAttachment;
					break;
					case RendererOutputType::Window:
						final_layout = vk2::ImageLayout::Present;
					break;
					default:
						tz_error("Unknown RendererOutputType. Please submit a bug report.");
					break;
				}

				vk2::RenderPassBuilder rbuilder;
				rbuilder.set_device(*info.device);
				rbuilder.with_attachment
				({
					.format = this->output_views.front().get_image().get_format(),
					.initial_layout = vk2::ImageLayout::Undefined,
					.final_layout = final_layout
				});

				vk2::SubpassBuilder sbuilder;
				sbuilder.set_pipeline_context(vk2::PipelineContext::Graphics);
				sbuilder.with_colour_attachment
				({
					.attachment_idx = 0,
					.current_layout = vk2::ImageLayout::ColourAttachment
				});

				this->render_pass = rbuilder.with_subpass(sbuilder.build()).build();
			}
			// Now we can setup our Framebuffers. Each framebuffer will only attach to a single image, so if a Swapchain is involved under-the-hood then we are likely to have multiple of these. However in Headless contexts we expect only a single framebuffer.
			for(vk2::ImageView& output_view : this->output_views)
			{
				this->output_framebuffers.push_back
				(vk2::FramebufferInfo{
					.render_pass = &this->render_pass,
					.attachments = {&output_view},
					.dimensions = output_view.get_image().get_dimensions()
				});
			}
		}

		std::span<vk2::Framebuffer> IOManager::get_output_framebuffers()
		{
			return this->output_framebuffers;
		}

		std::span<const vk2::Framebuffer> IOManager::get_output_framebuffers() const
		{
			return this->output_framebuffers;
		}

		const vk2::RenderPass& IOManager::get_render_pass() const
		{
			return this->render_pass;
		}

		tz::Vec2 IOManager::get_output_dimensions() const
		{
			tz_assert(!this->output_views.empty(), "IOManager had no output views, so impossible to decipher viewport dimensions. Please submit a bug report.");
			tz::Vec2ui dims = this->output_views.front().get_image().get_dimensions();
			return {static_cast<float>(dims[0]), static_cast<float>(dims[1])};
		}

		ResourceManager::ResourceManager(ResourceManagerInfo info):
		buffer_components(),
		texture_components(),
		texture_views(),
		basic_sampler(vk2::SamplerInfo
		{
			.device = info.device,
			.min_filter = vk2::LookupFilter::Nearest,
			.mag_filter = vk2::LookupFilter::Nearest,
			.mipmap_mode = vk2::MipLookupFilter::Nearest,
			.address_mode_u = vk2::SamplerAddressMode::ClampToEdge,
			.address_mode_v = vk2::SamplerAddressMode::ClampToEdge,
			.address_mode_w = vk2::SamplerAddressMode::ClampToEdge,
		}),
		descriptor_layout(vk2::DescriptorLayout::null()),
		descriptor_pool(vk2::DescriptorPool::null()),
		descriptors()
		{
			if(info.buffer_resources.empty() && info.texture_resources.empty())
			{
				return;
			}
			// Firstly create the resources as needed.
			for(const IResource* const buffer_resource : info.buffer_resources)
			{
				tz_assert(buffer_resource->get_type() == ResourceType::Buffer, "IResource was not a BufferResource as expected. Please submit a bug report.");
				// If the buffer resource is DynamicFixed, then we are going to make it CPU Persistent mapped, otherwise it's gonna be GPU and need a staging buffer later on.
				vk2::MemoryResidency residency;
				switch(buffer_resource->data_access())
				{
					case tz::gl::RendererInputDataAccess::StaticFixed:
						residency = vk2::MemoryResidency::GPU;
					break;
					case tz::gl::RendererInputDataAccess::DynamicFixed:
						residency = vk2::MemoryResidency::CPUPersistent;
					break;
					default:
						tz_error("Unrecognised RendererInputDataAccess. Please submit a bug report.");
						residency = vk2::MemoryResidency::GPU;
					break;
				}
				this->buffer_components.emplace_back
				(vk2::BufferInfo{
					.device = info.device,
					.size_bytes = buffer_resource->get_resource_bytes().size_bytes(),
					// TODO: Don't hardcode buffer resources to Storage Buffers.
					.usage = {vk2::BufferUsage::StorageBuffer, vk2::BufferUsage::TransferDestination},
					.residency = residency
				});
			}

			for(const IResource* const texture_resource : info.texture_resources)
			{
				const TextureResource* const tex_res = static_cast<const TextureResource* const>(texture_resource);
				tz_assert(texture_resource->get_type() == ResourceType::Texture, "IResource was not a TextureResource as expected. Please submit a bug report.");
				this->texture_components.emplace_back
				(vk2::ImageInfo{
					.device = info.device,
					.format = to_vk2(tex_res->get_format()),
					.dimensions = {tex_res->get_width(), tex_res->get_height()},
					.usage = {vk2::ImageUsage::SampledImage, vk2::ImageUsage::TransferDestination},
					.residency = vk2::MemoryResidency::GPU
				});
			}

			// Now populate the image views.
			for(vk2::Image& image : this->texture_components)
			{
				this->texture_views.emplace_back
				(vk2::ImageViewInfo{
					.image = &image,
					.aspect = vk2::ImageAspect::Colour
				});
			}
			
			// Figure out Descriptor stuffs. We are going to use bindless here.
			{
				vk2::DescriptorLayoutBuilder lbuilder;
				lbuilder.set_device(*info.device);
				// Each buffer gets their own binding id.
				for(std::size_t i = 0; i < info.buffer_resources.size(); i++)
				{
					lbuilder.with_binding
					({
						.type = vk2::DescriptorType::StorageBuffer,
						.count = 1
					});
				}
				// And one giant descriptor array for all textures. Wow.
				lbuilder.with_binding
				({
					.type = vk2::DescriptorType::ImageWithSampler,
					.count = static_cast<std::uint32_t>(info.texture_resources.size()),
					.flags = {vk2::DescriptorFlag::PartiallyBound}
				});
				this->descriptor_layout = lbuilder.build();
			}

			this->descriptor_pool = 
			{vk2::DescriptorPoolInfo{
				.limits =
				{
					.limits = 
					{
						{vk2::DescriptorType::StorageBuffer, info.buffer_resources.size() * info.frame_in_flight_count},
						{vk2::DescriptorType::ImageWithSampler, info.texture_resources.size() * info.frame_in_flight_count}
					},
					.max_sets = static_cast<std::uint32_t>(info.frame_in_flight_count)
				},
				.logical_device = info.device
			}};
			{
				tz::BasicList<const vk2::DescriptorLayout*> alloc_layout_list;
				for(std::size_t i = 0; i < info.frame_in_flight_count; i++)
				{
					alloc_layout_list.add(&this->descriptor_layout);
				}
				this->descriptors = this->descriptor_pool.allocate_sets
				({
					.set_layouts = std::move(alloc_layout_list)
				});
			};
			tz_assert(this->descriptors.success(), "Descriptor Pool allocation failed. Please submit a bug report.");

			// Now write the initial resources into their descriptors.
			vk2::DescriptorPool::UpdateRequest update = this->descriptor_pool.make_update_request();
			// For each set, make the same edits.
			for(std::size_t i = 0 ; i < info.frame_in_flight_count; i++)
			{
				vk2::DescriptorSet& set = this->descriptors.sets[i];
				vk2::DescriptorSet::EditRequest set_edit = set.make_edit_request();
				// Now update each binding corresponding to a buffer resource.
				for(std::size_t j = 0; j < info.buffer_resources.size(); j++)
				{
					set_edit.set_buffer(j,
					{
						.buffer = &this->buffer_components[j],
						.buffer_offset = 0,
						.buffer_write_size = this->buffer_components[j].size()
					});
				}
				// And finally the binding corresponding to the texture resource descriptor array
				for(std::size_t j = 0; j < info.texture_resources.size(); j++)
				{
					set_edit.set_image(info.buffer_resources.size(),
					{
						.sampler = &this->basic_sampler,
						.image_view = &this->texture_views[j]
					}, j);
				}
				update.add_set_edit(set_edit);
			}
			this->descriptor_pool.update_sets(update);
		}

		const vk2::DescriptorLayout& ResourceManager::get_descriptor_layout() const
		{
			return this->descriptor_layout;
		}

		GraphicsPipelineManager::GraphicsPipelineManager(GraphicsPipelineManagerInfo info):
		pipeline_layout(this->make_pipeline_layout(*info.descriptor_layout, info.frame_in_flight_count)),
		graphics_pipeline
		({
			.shaders = info.shader_program.native_data(),
			.state = vk2::PipelineState
			{
				.viewport = vk2::create_basic_viewport(info.viewport_dimensions)
			},
			.pipeline_layout = &this->pipeline_layout,
			.render_pass = &info.render_pass,
			.device = &info.render_pass.get_device()
		})
		{}

		vk2::PipelineLayout GraphicsPipelineManager::make_pipeline_layout(const vk2::DescriptorLayout& descriptor_layout, std::size_t frame_in_flight_count)
		{
		std::vector<const vk2::DescriptorLayout*> layout_ptrs(frame_in_flight_count, &descriptor_layout);
		return
		{{
			.descriptor_layouts = std::move(layout_ptrs),
			.logical_device = &descriptor_layout.get_device()
		}};
		}

		CommandProcessor::CommandProcessor(CommandProcessorInfo info):
		graphics_queue(info.device->get_hardware_queue
		({
			.field = {vk2::QueueFamilyType::Graphics},
			.present_support = (info.output_type == RendererOutputType::Window) ? true : false
		})),
		command_pool
		({
			.queue = this->graphics_queue
		}),
		commands(this->command_pool.allocate_buffers
		({
			.buffer_count = static_cast<std::uint32_t>(info.frame_in_flight_count + 1)
		})),
		frame_in_flight_count(info.frame_in_flight_count)
		{
			tz_assert(this->commands.success(), "Failed to allocate from CommandPool");
		}

		std::span<const vk2::CommandBuffer> CommandProcessor::get_render_command_buffers() const
		{
			return {this->commands.buffers.begin(), this->frame_in_flight_count};
		}

		std::span<vk2::CommandBuffer> CommandProcessor::get_render_command_buffers()
		{
			return {this->commands.buffers.begin(), this->frame_in_flight_count};
		}
	}

	RendererVulkan2::RendererVulkan2(RendererBuilderVulkan2 builder, RendererBuilderDeviceInfoVulkan2 device_info):
	RendererBase(builder),
	vk_device(*device_info.device),
	io_manager
	({
		.device = device_info.device,
		.output_images = device_info.output_images,
		.output_type = builder.get_output()->get_type()
	}),
	resource_manager
	(detail::ResourceManagerInfo{
		.device = device_info.device,
		.buffer_resources = builder.get_resources(ResourceType::Buffer),
		.texture_resources = builder.get_resources(ResourceType::Texture),
		.frame_in_flight_count = device_info.output_images.size()
	}),
	graphics_pipeline_manager
	({
		.descriptor_layout = &this->resource_manager.get_descriptor_layout(),
		.shader_program = static_cast<const ShaderVulkan2&>(builder.get_shader()).vk_get_shader(),
		.render_pass = this->io_manager.get_render_pass(),
		.frame_in_flight_count = device_info.output_images.size(),
		.viewport_dimensions = this->io_manager.get_output_dimensions()
	}),
	command_processor
	({
	 	.device = &this->vk_device,
		.frame_in_flight_count = device_info.output_images.size(),
		.output_type = builder.get_output()->get_type(),
		.output_framebuffers = this->io_manager.get_output_framebuffers()
	})
	{
		this->command_processor.do_scratch_operations([](vk2::CommandBufferRecording& recording)
		{
			int x = 5;
		});
	}
}

#endif // TZ_VULKAN
