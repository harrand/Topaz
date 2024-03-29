#include "tz/gl/impl/vulkan/detail/hardware/physical_device.hpp"
#include "tz/gl/impl/vulkan/detail/logical_device.hpp"
#include "tz/gl/impl/vulkan/detail/sampler.hpp"
#include "tz/gl/impl/vulkan/detail/swapchain.hpp"
#include "tz/gl/impl/vulkan/detail/render_pass.hpp"
#include "tz/gl/impl/vulkan/detail/graphics_pipeline.hpp"
#include "tz/gl/impl/vulkan/detail/framebuffer.hpp"
#include "tz/gl/impl/vulkan/detail/command.hpp"
#include "tz/gl/impl/vulkan/detail/semaphore.hpp"
#include "tz/gl/impl/vulkan/detail/fence.hpp"

#include "vk2_triangle_demo_common.hpp"
#include "tz/core/matrix_transform.hpp"
#include <random>
#include <chrono>

// Like vk2_triangle_demo_small, but using extra features:
// - More than one triangle
// - Storage buffers for vertex data & position/rotation/scale
// - Textures generated from randomness.

struct TriangleResourceData
{
	tz::vec3 position;
	float pad0;
	tz::vec3 rotation;
	float pad1;
	tz::vec3 scale;
	float pad2;
	tz::mat4 mvp;
	std::uint32_t texture_id;
	std::uint32_t pad3[3];
};

struct TriangleVertexData
{
	tz::vec3 position;
	float pad0;
	tz::vec2 texcoord;
	float pad1[2];
};

constexpr std::size_t triangle_count = 3;

int main()
{
	tz::initialise
	({
		.name = "vk2_triangle_demo_large",
		.window = {.flags = {.resizeable = false}}
	});
	{
		// First start with basic state (get a LogicalDevice and Swapchain running)
		using namespace tz::gl::vk2;
		PhysicalDevice pdev = get_all_devices().front();

		LogicalDevice ldev
		{{
			.physical_device = pdev,
			.extensions = {DeviceExtension::Swapchain, DeviceExtension::ShaderDebugPrint},
			.features = {DeviceFeature::BindlessDescriptors}
		}};
		PhysicalDeviceSurfaceCapabilityInfo pdev_capability = pdev.get_surface_capabilities();

		image_format swapchain_format = pdev.get_supported_surface_formats().front();

		Swapchain swapchain
		{{

			.device = &ldev,
			.swapchain_image_count_minimum = pdev_capability.min_image_count,
			.image_format = swapchain_format,
			.present_mode = present_traits::get_mandatory_present_modes().front(),
		}};

		// Create two storage buffers.
		// Firstly, one for vertex data (this is basically a pseudo-vertex-buffer).
		Buffer vertex_storage_buffer
		{{
			.device = &ldev,
			.size_bytes = sizeof(TriangleVertexData) * 3,
			.usage = {BufferUsage::StorageBuffer, BufferUsage::TransferDestination},
			.residency = MemoryResidency::GPU
		}};
		// Then one for resource data (model matrix components basically)
		Buffer triangle_resource_buffer
		{{
			.device = &ldev,
			.size_bytes = triangle_count * sizeof(TriangleResourceData),
			.usage = {BufferUsage::StorageBuffer},
			.residency = MemoryResidency::CPUPersistent
		}};
		// Finally, two textures just for fun. We're gonna use a descriptor array!
		Image on_texture
		{{
			.device = &ldev,
			.format = image_format::RGBA32,
			.dimensions = {32u, 32u},
			.usage = {ImageUsage::SampledImage, ImageUsage::TransferDestination},
			.residency = MemoryResidency::GPU
		}};
		// Texture will need a view and sampler.
		ImageView on_texture_view
		{{
			.image = &on_texture,
			.aspect = ImageAspectFlag::Colour
		}};
		Image off_texture
		{{
			.device = &ldev,
			.format = image_format::RGBA32,
			.dimensions = {32u, 32u},
			.usage = {ImageUsage::SampledImage, ImageUsage::TransferDestination},
			.residency = MemoryResidency::GPU
		}};
		// Texture will need a view and sampler.
		ImageView off_texture_view
		{{
			.image = &off_texture,
			.aspect = ImageAspectFlag::Colour
		}};
		Sampler basic_sampler
		{{
			.device = &ldev,
			.min_filter = LookupFilter::Nearest,
			.mag_filter = LookupFilter::Nearest,
			.mipmap_mode = MipLookupFilter::Nearest,
			.address_mode_u = SamplerAddressMode::ClampToEdge,
			.address_mode_v = SamplerAddressMode::ClampToEdge,
			.address_mode_w = SamplerAddressMode::ClampToEdge,
		}};
		// Create RenderPass.
		// Pass consists of 1 attachment (swapchain image, initial layout: Undefined, final layout: Present)
		// 1 subpass (attachment 0: Undefined -> Colour Attachment)
		RenderPassBuilder rbuilder;
		rbuilder.set_device(ldev);
		rbuilder.with_attachment
		({
			.format = swapchain_format,
			.initial_layout = ImageLayout::Undefined,
			.final_layout = ImageLayout::Present
		});
		
		SubpassBuilder sbuilder;
		sbuilder.set_pipeline_context(PipelineContext::graphics);
		sbuilder.with_colour_attachment
		({
			.attachment_idx = 0,
			.current_layout = ImageLayout::ColourAttachment
		});

		rbuilder.with_subpass(sbuilder.build());
		RenderPass render_pass{rbuilder.build()};

		// Now create GraphicsPipeline.
		DescriptorLayoutBuilder lbuilder;
		lbuilder.set_device(ldev);
		lbuilder.with_binding
		({
		 	// Vertex Storage Buffer
			.type = DescriptorType::StorageBuffer,
			.count = 1
		}).with_binding
		({
			// Triangle resource Buffer
			.type = DescriptorType::StorageBuffer,
			.count = 1
		}).with_binding
		({
		 	// Both the 'on' and 'off' textures have corresponding descriptors at this binding id. Note that the array is actually of size 3. 'on' = 0, 'off' = 1 and the last index is purposefully unused (just to ensure that PartiallyBound works, nobody tries to use this texture id).
			.type = DescriptorType::ImageWithSampler,
			.count = 3,
			.flags = {DescriptorFlag::PartiallyBound}
		});
		DescriptorLayout dlayout{lbuilder.build()};
		// Now let's create the Descriptor Sets.
		DescriptorPool dpool
		{{
			.limits =
			{
				.limits =
				{
					{DescriptorType::StorageBuffer, swapchain.get_images().size() * 2},
					{DescriptorType::ImageWithSampler, swapchain.get_images().size() * 2}
				},
				.max_sets = static_cast<std::uint32_t>(swapchain.get_images().size())
			},
			.logical_device = &ldev,
		}};
		tz::basic_list<const DescriptorLayout*> alloc_layout_list;
		for(std::size_t i = 0; i < swapchain.get_images().size(); i++)
		{
			alloc_layout_list.add(&dlayout);
		}
		DescriptorPool::AllocationResult dpool_alloc = dpool.allocate_sets
		({
			.set_layouts = std::move(alloc_layout_list)
		});
		tz::assert(dpool_alloc.success(), "Descriptor Pool allocation failed.");
		tz::assert(dpool_alloc.sets.length() == swapchain.get_images().size(), "Descriptor Pool allocation returned incorrect number of DescriptorSets");
		// Write the buffers and texture to the sets. We only need to do this once fortunately.
		// We have a set per swapchain image. For each of these sets:
		// 	- Write the vertex storage buffer to binding 0
		// 	- Write the triangle resource buffer to binding 1
		// 	- Write the triangle texture to binding 2
		DescriptorPool::UpdateRequest update = dpool.make_update_request();
		for(std::size_t i = 0; i < swapchain.get_images().size(); i++)
		{
			DescriptorSet& set = dpool_alloc.sets[i];
			DescriptorSet::EditRequest set_edit = set.make_edit_request();
			set_edit.set_buffer(0,
			{
				.buffer = &vertex_storage_buffer,
				.buffer_offset = 0,
				.buffer_write_size = vertex_storage_buffer.size()
			});
			set_edit.set_buffer(1,
			{
				.buffer = &triangle_resource_buffer,
				.buffer_offset = 0,
				.buffer_write_size = triangle_resource_buffer.size()
			});
			set_edit.set_image(2,
			{
				.sampler = &basic_sampler,
				.image_view = &on_texture_view
			}, 0);
			set_edit.set_image(2,
			{
				.sampler = &basic_sampler,
				.image_view = &off_texture_view
			}, 1);
			update.add_set_edit(set_edit);
		}
		dpool.update_sets(update);

		// TODO: Sanity check.
		std::vector<const DescriptorLayout*> layout_ptrs;
		for(std::size_t i = 0; i < swapchain.get_images().size(); i++)
		{
			layout_ptrs.push_back(&dlayout);
		}
		PipelineLayout pipeline_layout
		{{
			.descriptor_layouts = layout_ptrs,
			.logical_device = &ldev
		}};

		Shader shader
		{{
			.device = &ldev,
			.modules =
			{
				ShaderModuleInfo
				{
					.device = &ldev,
					.type = ShaderType::vertex,
					.code = read_shader_code("demo/gl/vk2/shaders/triangle_demo_large.vertex.tzsl.spv")
				},
				
				ShaderModuleInfo
				{
					.device = &ldev,
					.type = ShaderType::fragment,
					.code = read_shader_code("demo/gl/vk2/shaders/triangle_demo_large.fragment.tzsl.spv")
				}
			}
		}};

		GraphicsPipeline graphics_pipeline
		{{
			.shaders = shader.native_data(),
			.state = PipelineState
			{
				.viewport = create_basic_viewport({tz::window().get_width(), tz::window().get_height()})

			},
			.pipeline_layout = &pipeline_layout,
			.render_pass = &render_pass,

			.device = &ldev
		}};

		// Create framebuffers from swapchain images.
		std::vector<Framebuffer> swapchain_framebuffers;
		swapchain_framebuffers.reserve(swapchain.get_image_views().size());
		for(ImageView& swapchain_image_view : swapchain.get_image_views())
		{
			swapchain_framebuffers.emplace_back
			(FramebufferInfo{
				.render_pass = &render_pass,
				.attachments = {&swapchain_image_view},
				.dimensions = {swapchain_image_view.get_image().get_dimensions()}
			});
		}

		// Create and record command buffers. One command buffer per swapchain image
		// Retrieve a hardware queue which we can use.
		hardware::Queue* queue = ldev.get_hardware_queue
		({
			.field = {QueueFamilyType::graphics},
			.present_support = true
		});

		CommandPool cpool
		{{
			.queue = queue
		}};
		CommandPool::AllocationResult result = cpool.allocate_buffers
		({
		 	// Enough buffers for each swapchain image, plus an extra for one-time commands.
			.buffer_count = static_cast<std::uint32_t>(swapchain.get_images().size()) + 1
		});
		tz::assert(result.success(), "CommandPool allocation failed unexpectedly");
		tz::assert(result.buffers.length() == swapchain.get_images().size() + 1, "Did not allocate correct number of CommandBuffers");

		for(std::size_t i = 0; i < swapchain.get_images().size(); i++)
		{
			CommandBuffer& command_buf = result.buffers[i];
			CommandBufferRecording recording = command_buf.record();
			{
				CommandBufferRecording::RenderPassRun run{swapchain_framebuffers[i], recording};
				recording.bind_pipeline
				({
					.pipeline = &graphics_pipeline,
					.pipeline_context = PipelineContext::graphics
				});
				tz::basic_list<const DescriptorSet*> sets;
				sets.resize(dpool_alloc.sets.length());
				std::transform(dpool_alloc.sets.begin(), dpool_alloc.sets.end(), sets.begin(), [](const DescriptorSet& set){return &set;});

				recording.bind_descriptor_sets
				({
					.pipeline_layout = &pipeline_layout,
					.context = PipelineContext::graphics,
					.descriptor_sets = sets,
					.first_set_id = 0
				});
				recording.draw
				({
					.vertex_count = 3 * triangle_count,
					.instance_count = 1,
					.first_vertex = 0,
					.first_instance = 0
				});
			}
		}
		// Now record our one-time commands.
		{
			CommandBuffer& cmd = result.buffers.back();
			// Right now our buffers and image has no data. Let's fill 'em up.
			// Start with vertex storage buffer.
			Buffer vertex_storage_buffer_staging
			{{
				.device = &ldev,
				.size_bytes = vertex_storage_buffer.size(),
				.usage = {BufferUsage::TransferSource},
				.residency = MemoryResidency::CPU
			}};
			// resource buffer is CPUPersistent, so we don't need to do any scratch commands for it.
			// Now a buffer which we can transfer image data from is needed.
			// RGBA32_UInt, so four uint8_ts per element. 32x32 elements
			Buffer on_texture_staging_buffer
			{{
				.device = &ldev,
				.size_bytes = (32/8) * (32*32),
				.usage = {BufferUsage::TransferSource},
				.residency = MemoryResidency::CPU
			}};
			Buffer on_texture_staging_buffer2
			{{
				.device = &ldev,
				.size_bytes = (32/8) * (32*32),
				.usage = {BufferUsage::TransferSource},
				.residency = MemoryResidency::CPU
			}};

			// Write the data into staging buffers
			// Starting again with the vertex data.
			{
				std::span<TriangleVertexData> vertices = vertex_storage_buffer_staging.map_as<TriangleVertexData>();
				vertices[0] =
				{
					.position = {-0.5f, -0.5f, 0.0f},
					.texcoord = {1.0f, 0.0f}
				};
				vertices[1] = 
				{
					.position = {0.5f, -0.5f, 0.0f},
					.texcoord = {0.0f, 0.0f}
				};
				vertices[2] = 
				{
					.position = {0.5f, 0.5f, 0.0f},
					.texcoord = {0.0f, 1.0f}
				};
				vertex_storage_buffer_staging.unmap();
			}
			// Finally, the texture data.
			{
				// Remember the texture is 32x32, where each element is uint8*4_t
				std::span<std::uint32_t> texture_data = on_texture_staging_buffer.map_as<std::uint32_t>();
				std::default_random_engine rand(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
				for(std::uint32_t& texel : texture_data)
				{
					texel = rand();
				}
				on_texture_staging_buffer.unmap();
			}
			{
				std::span<std::uint32_t> texture_data = on_texture_staging_buffer2.map_as<std::uint32_t>();
				std::span<std::uint32_t> old_texture_data = on_texture_staging_buffer.map_as<std::uint32_t>();
				for(std::size_t i = 0; i < texture_data.size(); i++)
				{
					// wtf this is ABGR?
					std::uint32_t& texel = texture_data[i];
					texel = old_texture_data[i];
					texel |= 0xFF000000;
					texel &= 0xFF111133;

				}
				on_texture_staging_buffer.unmap();
				on_texture_staging_buffer2.unmap();
			}

			// We made an extra command buffer earlier. Time to use it to submit these transfers to the GPU buffers/image.
			{
				CommandBufferRecording recording = cmd.record();
				recording.buffer_copy_buffer
				({
					.src = &vertex_storage_buffer_staging,
					.dst = &vertex_storage_buffer
				});
				// Image starts in Undefined layout, which can't be a transfer destination. Need to transition the image layout. This is gonna hurt...
				recording.transition_image_layout
				({
					.image = &on_texture,
					.target_layout = ImageLayout::TransferDestination,
					.source_access = {AccessFlag::None},
					.destination_access = {AccessFlag::TransferOperationWrite},
					.source_stage = PipelineStage::Top,
					.destination_stage = PipelineStage::TransferCommands,
					.image_aspects = {ImageAspectFlag::Colour}
				});
				recording.buffer_copy_image
				({
					.src = &on_texture_staging_buffer,
					.dst = &on_texture,
					.image_aspects = {ImageAspectFlag::Colour}
				});
				// We're about the use the image as a shader resource, so we'll transition it back.
				recording.transition_image_layout
				({
					.image = &on_texture,
					.target_layout = ImageLayout::ShaderResource,
					.source_access = {AccessFlag::TransferOperationWrite},
					.destination_access = {AccessFlag::ShaderResourceRead},
					.source_stage = PipelineStage::TransferCommands,
					.destination_stage = PipelineStage::FragmentShader,
					.image_aspects = {ImageAspectFlag::Colour}
				});
				// same shit for 2nd image
				recording.transition_image_layout
				({
					.image = &off_texture,
					.target_layout = ImageLayout::TransferDestination,
					.source_access = {AccessFlag::None},
					.destination_access = {AccessFlag::TransferOperationWrite},
					.source_stage = PipelineStage::Top,
					.destination_stage = PipelineStage::TransferCommands,
					.image_aspects = {ImageAspectFlag::Colour}
				});
				recording.buffer_copy_image
				({
					.src = &on_texture_staging_buffer2,
					.dst = &off_texture,
					.image_aspects = {ImageAspectFlag::Colour}
				});
				// We're about the use the image as a shader resource, so we'll transition it back.
				recording.transition_image_layout
				({
					.image = &off_texture,
					.target_layout = ImageLayout::ShaderResource,
					.source_access = {AccessFlag::TransferOperationWrite},
					.destination_access = {AccessFlag::ShaderResourceRead},
					.source_stage = PipelineStage::TransferCommands,
					.destination_stage = PipelineStage::FragmentShader,
					.image_aspects = {ImageAspectFlag::Colour}
				});
			}
			Fence work_complete_fence
			{{
				.device = &ldev,
			}};
			// Do the transfers and halt until it's all done.
			queue->submit
			({
				.command_buffers = {&cmd},
				.waits = {},
				.signal_semaphores = {},
				.execution_complete_fence = &work_complete_fence
			});
			work_complete_fence.wait_until_signalled();
		}

		// Now create sync objects for frame administration.
		std::vector<BinarySemaphore> image_available_semaphores;
		std::vector<BinarySemaphore> render_finished_semaphores;
		std::vector<Fence> in_flight_fences;
		std::vector<const Fence*> images_in_flight(max_frames_in_flight, nullptr); 
		for(std::size_t i = 0; i < max_frames_in_flight; i++)
		{
			image_available_semaphores.emplace_back(ldev);
			render_finished_semaphores.emplace_back(ldev);
			in_flight_fences.emplace_back(FenceInfo
			{
				.device = &ldev,
				.initially_signalled = true
			});
		}
		// Now the resource data.
		std::span<TriangleResourceData> resources = triangle_resource_buffer.map_as<TriangleResourceData>();
		auto set_triangle_data = [&resources](std::size_t triangle_idx, tz::vec3 position, tz::vec3 rotation, tz::vec3 scale, std::uint32_t texture_id = 0)
		{
			tz::assert(texture_id < 2, "Texture ID %u is invalid.", texture_id);
			resources[triangle_idx] =
			{
				.position = position,
				.rotation = rotation,
				.scale = scale,
				.mvp = {},
				.texture_id = texture_id
			};
			resources[triangle_idx].position[0] = (0.5f * triangle_idx) - 0.5f;
			tz::mat4 m = tz::model(resources[triangle_idx].position, resources[triangle_idx].rotation, resources[triangle_idx].scale);

			tz::mat4 v = tz::view({0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f});
			tz::mat4 p = tz::perspective(1.27f, tz::window().get_width() / tz::window().get_height(), 0.1f, 1000.0f);
			resources[triangle_idx].mvp = (p * v * m).transpose();
		};

		auto set_on_texture_id = [&resources](std::size_t triangle_index, std::uint32_t texture_id)
		{
			resources[triangle_index].texture_id = texture_id;
		};
		for(std::size_t i = 0; i < triangle_count; i++)
		{
			set_triangle_data(i,
				{0.0f,  0.0f, -0.2f},
				{0.0f, 0.0f, i * 1.5708f},
				{0.4f, 0.4f, 0.4f}, 1);
		}

		// Main game loop

		std::uint32_t swapchain_image_index;
		std::size_t current_frame = 0;

		while(!tz::window().is_close_requested())
		{
			static float counter = 0.0f;
			counter += 0.05f;
			if(counter >= 100.0f)
			{
				counter = 0;
			}
			std::size_t target_triangle_index = static_cast<std::size_t>((counter / 100.0f) * triangle_count);
			set_triangle_data(target_triangle_index,
				{0.0f, std::sin(counter) * 0.3f, -0.2f},
				{0.0f, counter / 1.5f, counter / 1.5f},
				{0.4f, 0.4f, 0.4f}, 0);
			for(std::size_t i = 0; i < triangle_count; i++)
			{
				if(i != target_triangle_index)
				{
					set_on_texture_id(i, 1);
				}
			}

			// Frame admin stuff
			in_flight_fences[current_frame].wait_until_signalled();

			swapchain_image_index = swapchain.acquire_image
			({
				.signal_semaphore = &image_available_semaphores[current_frame],
			}).image_index;

			if(images_in_flight[swapchain_image_index] != nullptr)
			{
				images_in_flight[swapchain_image_index]->wait_until_signalled();
			}
			images_in_flight[swapchain_image_index] = &in_flight_fences[swapchain_image_index];

			in_flight_fences[current_frame].unsignal();
			queue->submit
			({
				.command_buffers = {&result.buffers[swapchain_image_index]},
				.waits = 
				{
					hardware::Queue::SubmitInfo::WaitInfo
					{
						.wait_semaphore = &image_available_semaphores[current_frame],
						.wait_stage = PipelineStage::ColourAttachmentOutput
					}
				},
				.signal_semaphores = {&render_finished_semaphores[current_frame]},
				.execution_complete_fence = &in_flight_fences[current_frame]
			});

			hardware::Queue::PresentResult res = queue->present
			({
			 	.wait_semaphores = {&render_finished_semaphores[current_frame]},
				.swapchain = &swapchain,
				.swapchain_image_index = swapchain_image_index
			});
			tz::assert(res == hardware::Queue::PresentResult::Success || res == hardware::Queue::PresentResult::Success_Suboptimal, "Presentation failed.");

			current_frame = (current_frame + 1) % max_frames_in_flight;

			tz::window().update();
		}
		ldev.wait_until_idle();
	}
	tz::terminate();
}
