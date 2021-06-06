#if !TZ_VULKAN
    static_assert(false, "Cannot build vk_init_demo with TZ_VULKAN disabled.");
#endif

#include "core/tz.hpp"
#include "core/assert.hpp"
#include "core/report.hpp"
#include "gl/vk/setup/vulkan_instance.hpp"
#include "gl/vk/hardware/device.hpp"
#include "gl/vk/hardware/device_filter.hpp"
#include "gl/vk/logical_device.hpp"
#include "gl/vk/swapchain.hpp"
#include "gl/vk/pipeline/graphics_pipeline.hpp"
#include "gl/vk/pipeline/shader_compiler.hpp"

#include "gl/vk/render_pass.hpp"
#include "gl/vk/framebuffer.hpp"
#include "gl/vk/command.hpp"
#include "gl/vk/semaphore.hpp"
#include "gl/vk/present.hpp"
#include "gl/vk/submit.hpp"
#include "gl/vk/frame_admin.hpp"
#include "gl/vk/descriptor.hpp"

#include "core/matrix_transform.hpp"

#include <chrono>

using namespace tz::gl;

struct Vertex
{
    tz::Vec3 pos;
    tz::Vec3 colour;
    tz::Vec2 tex_coord;
};

static constexpr vk::VertexBindingDescription binding_description{0, sizeof(Vertex), vk::VertexInputRate::PerVertexBasis};
static constexpr vk::VertexAttributeDescription pos_description{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)};
static constexpr vk::VertexAttributeDescription col_description{0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, colour)};
static constexpr vk::VertexAttributeDescription texcoord_description{0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, tex_coord)};

struct MVP
{
    tz::Mat4 m;
    tz::Mat4 v;
    tz::Mat4 p;
};

int main()
{
    constexpr tz::GameInfo vk_squares_demo{"vk_squares_demo", tz::EngineInfo::Version{1, 0, 0}, tz::info()};
    tz_report("%s", vk_squares_demo.to_string().c_str());
    
    std::array<Vertex, 8> vertices =
    {
        Vertex{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        Vertex{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        Vertex{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        Vertex{{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},

        Vertex{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        Vertex{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        Vertex{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        Vertex{{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}
    };

    std::array<unsigned int, 12> indices = 
    {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4
    };

    std::array<unsigned char, 16> imgdata = 
    {
        0b0000'0000,
        0b0000'0000,
        0b1111'1111,
        0b1111'1111,

        0b1111'1111,
        0b0000'0000,
        0b0000'0000,
        0b1111'1111,

        0b0000'0000,
        0b1111'1111,
        0b0000'0000,
        0b1111'1111,

        0b0000'0000,
        0b0000'0000,
        0b1111'1111,
        0b1111'1111,
    };

    tz::initialise(vk_squares_demo);
    {
        vk::hardware::DeviceList valid_devices = tz::gl::vk::hardware::get_all_devices();
        vk::hardware::QueueFamilyTypeField type_requirements{{vk::hardware::QueueFamilyType::Graphics, vk::hardware::QueueFamilyType::Present}};
        // Let's grab a device which can do graphics and present images.
        // It also must support the swapchain device extension.
        {
            namespace hw = vk::hardware;
            hw::DeviceFilterList filters;
            filters.emplace<hw::DeviceQueueFamilyFilter>(type_requirements);
            filters.emplace<hw::DeviceExtensionSupportFilter>(std::initializer_list<vk::VulkanExtension>{"VK_KHR_swapchain"});
            filters.filter_all(valid_devices);
        }
        tz_assert(!valid_devices.empty(), "No valid devices. Require a physical device which supports graphics and present queue families.");
        // Just choose the first one.
        vk::hardware::Device my_device = valid_devices.front();
        vk::hardware::MemoryProperties my_memory_props = my_device.get_memory_properties();
        std::optional<vk::hardware::DeviceQueueFamily> maybe_my_qfam = std::nullopt;
        for(auto fam : my_device.get_queue_families())
        {
            if(fam.types_supported.contains(type_requirements))
            {
                maybe_my_qfam = fam;
            }
        }
        tz_assert(maybe_my_qfam.has_value(), "Valid device found which supports present and graphics, but not a single queue that can do both.");
        vk::hardware::DeviceQueueFamily my_qfam = maybe_my_qfam.value();

        // Now create the device which can do both. We also ask it to use the swapchain extension.
        vk::LogicalDevice my_logical_device{my_qfam, {"VK_KHR_swapchain"}};
        // Let's also check the extent to which this physical device supports swapchains
        vk::hardware::SwapchainSupportDetails swapchain_support = my_device.get_window_swapchain_support();
        tz_assert(swapchain_support.supports_swapchain, "Very odd. The logical device was spawned using swapchain, but the physical device apparantly doesn't support it after all? There's 99\% a bug somewhere");
        tz_debug_report("Swapchain : Image count range: %lu-%lu. %zu formats available. %zu present modes available.", swapchain_support.capabilities.minImageCount, swapchain_support.capabilities.maxImageCount, swapchain_support.formats.length(), swapchain_support.present_modes.length());
        
        // Using some very strict requirements here, some machines might straight-up not support this.
        vk::hardware::SwapchainSelectorPreferences my_prefs;
        my_prefs.format_pref = {vk::hardware::SwapchainFormatPreferences::Goldilocks, vk::hardware::SwapchainFormatPreferences::FlexibleGoldilocks, vk::hardware::SwapchainFormatPreferences::DontCare};
        my_prefs.present_mode_pref = {vk::hardware::SwapchainPresentModePreferences::PreferTripleBuffering, vk::hardware::SwapchainPresentModePreferences::DontCare};
        vk::Swapchain swapchain{my_logical_device, my_prefs};

        vk::pipeline::VertexInputState vertex_input_state
        {
            vk::VertexBindingDescriptions{binding_description},
            vk::VertexAttributeDescriptions{pos_description, col_description, texcoord_description}
        };

        vk::RenderPassBuilder builder;
        vk::Attachment col
        {
            swapchain.get_format(), /* AKA vk::Image::Format::Rgba32sRGB for my machine */
            vk::Attachment::LoadOperation::Clear,
            vk::Attachment::StoreOperation::Store,
            vk::Image::Layout::Undefined,
            vk::Image::Layout::Present
        };

        vk::Attachment depth
        {
            vk::Image::Format::DepthFloat32,
            vk::Attachment::LoadOperation::Clear,
            vk::Attachment::StoreOperation::DontCare,
            vk::Attachment::LoadOperation::DontCare,
            vk::Attachment::StoreOperation::DontCare,
            vk::Image::Layout::Undefined,
            vk::Image::Layout::DepthAttachment
        };

        builder.with(vk::Attachments{col, depth});
        vk::RenderPass simple_colour_pass{my_logical_device, builder};

        vk::LayoutBuilder layout_build;
        std::uint32_t ubo_binding = layout_build.add(vk::DescriptorType::UniformBuffer, vk::pipeline::ShaderTypeField{vk::pipeline::ShaderType::Vertex});
        std::uint32_t img_binding = layout_build.add(vk::DescriptorType::CombinedImageSampler, vk::pipeline::ShaderTypeField{vk::pipeline::ShaderType::Fragment});
        tz_assert(ubo_binding == 0, "Expected binding to be 0. Shader needs to change or this");
        vk::DescriptorSetLayout layout{my_logical_device, layout_build};
        
        vk::pipeline::Layout my_layout{my_logical_device, vk::DescriptorSetLayoutRefs{layout}};

        auto vtx_src = vk::read_external_shader(".\\demo\\gl\\vk\\basic.vertex.glsl").value();
        auto frg_src = vk::read_external_shader(".\\demo\\gl\\vk\\basic.fragment.glsl").value();
        vk::ShaderModule vertex{my_logical_device, {vtx_src.begin(), vtx_src.end()}};
        vk::ShaderModule fragment{my_logical_device, {frg_src.begin(), frg_src.end()}};

        vk::GraphicsPipeline my_pipeline
        {
            {vk::pipeline::ShaderStage{vertex, vk::pipeline::ShaderType::Vertex}, vk::pipeline::ShaderStage{fragment, vk::pipeline::ShaderType::Fragment}},
            my_logical_device,
            vertex_input_state,
            vk::pipeline::InputAssembly{vk::pipeline::PrimitiveTopology::Triangles},
            vk::pipeline::ViewportState{swapchain},
            vk::pipeline::RasteriserState
            {
                false,
                false,
                vk::pipeline::PolygonMode::Fill,
                1.0f,
                vk::pipeline::CullingStrategy::None
            },
            vk::pipeline::MultisampleState{},
            vk::pipeline::ColourBlendState{},
            vk::pipeline::DynamicState::None(),
            my_layout,
            simple_colour_pass
        };

        const std::size_t vertices_bytes = sizeof(Vertex) * vertices.size();
        const std::size_t indices_bytes = sizeof(unsigned int) * indices.size();
        const std::size_t mvp_bytes = sizeof(MVP);
        const std::size_t img_bytes = sizeof(imgdata);
        vk::Buffer buf{vk::BufferType::Vertex, vk::BufferPurpose::TransferDestination, my_logical_device, vk::hardware::MemoryResidency::GPU, vertices_bytes};
        vk::Buffer index_buf{vk::BufferType::Index, vk::BufferPurpose::TransferDestination, my_logical_device, vk::hardware::MemoryResidency::GPU, indices_bytes};
        vk::Buffer img_buf{vk::BufferType::Staging, vk::BufferPurpose::TransferSource, my_logical_device, vk::hardware::MemoryResidency::CPU, img_bytes};
        img_buf.write(imgdata.data(), img_bytes);
        std::vector<vk::Buffer> mvp_bufs;

        vk::Image img{my_logical_device, 2, 2, vk::Image::Format::Rgba32sRGB, {vk::Image::Usage::TransferDestination, vk::Image::Usage::Sampleable}, vk::hardware::MemoryResidency::GPU};
        vk::ImageView img_view{my_logical_device, img};
        vk::SamplerProperties props
        {
            .min_filter = VK_FILTER_LINEAR,
            .mag_filter = VK_FILTER_LINEAR,
            .address_mode_u = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .address_mode_v = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .address_mode_w = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        };
        vk::Sampler img_sampler{my_logical_device, props};

        vk::Image depth_img{my_logical_device, static_cast<std::uint32_t>(swapchain.get_width()), static_cast<std::uint32_t>(swapchain.get_height()), vk::Image::Format::DepthFloat32, {vk::Image::Usage::DepthStencilAttachment}, vk::hardware::MemoryResidency::GPU};
        vk::ImageView depth_img_view{my_logical_device, depth_img};
        
        for(std::size_t i = 0; i < swapchain.get_image_views().size(); i++)
        {
            mvp_bufs.emplace_back(vk::BufferType::Uniform, vk::BufferPurpose::NothingSpecial, my_logical_device, vk::hardware::MemoryResidency::CPU, mvp_bytes);
        }

        std::vector<vk::Framebuffer> swapchain_buffers;
        for(const vk::ImageView& swapchain_view : swapchain.get_image_views())
        {
            swapchain_buffers.emplace_back(simple_colour_pass, swapchain_view, depth_img_view, VkExtent2D{static_cast<std::uint32_t>(swapchain.get_width()), static_cast<std::uint32_t>(swapchain.get_height())});
        }

        vk::DescriptorPoolBuilder pool_builder;
        pool_builder
        // Enough for all our descriptors.
        .with_capacity(swapchain.get_image_views().size() * 2)
        // We have one UBO per swapchain image.
        .with_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, swapchain.get_image_views().size())
        .with_size(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, swapchain.get_image_views().size());
        for(std::size_t i = 0; i < swapchain.get_image_views().size(); i++)
        {
            pool_builder.with_layout(layout);
        }

        vk::DescriptorPool descriptor_pool{my_logical_device, pool_builder};
        vk::DescriptorSetsCreationRequests requests;
        for(std::size_t i = 0; i < swapchain.get_image_views().size(); i++)
        {
            vk::DescriptorSetsCreationRequest& request = requests.new_request();
            request.add_buffer(mvp_bufs[i], 0, VK_WHOLE_SIZE, ubo_binding);
            request.add_image(img_view, img_sampler, img_binding);
        }
        descriptor_pool.initialise_sets(requests);
        /*
        auto num_sets = static_cast<std::uint32_t>(swapchain.get_image_views().size());
        vk::DescriptorSetLayouts layouts2;
        for(std::size_t i = 0; i < swapchain.get_image_views().size(); i++)
        {
            layouts2.emplace_back(my_logical_device, vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::UniformBuffer});
        }
        vk::DescriptorPool descriptor_pool{my_logical_device, vk::DescriptorPoolSizes{{vk::DescriptorType::UniformBuffer, num_sets}}, std::move(layouts2)};
        descriptor_pool.with(num_sets, mvp_bufs);
        */

        vk::CommandPool command_pool(my_logical_device, my_qfam, vk::CommandPool::RecycleBuffer);
        command_pool.with(swapchain.get_image_views().size() + 1);
        for(std::size_t i = 0; i < swapchain.get_image_views().size(); i++)
        {
            vk::CommandBufferRecording render = command_pool[i].record();
            vk::RenderPassRun run{command_pool[i], simple_colour_pass, swapchain_buffers[i], swapchain.full_render_area(), VkClearValue{0.1f, 0.3f, 0.7f, 0.0f}};
            my_pipeline.bind(command_pool[i]);
            render.bind(buf);
            render.bind(index_buf);
            render.bind(descriptor_pool[i], my_layout);
            render.draw_indexed(indices.size());
        }

        vk::hardware::Queue graphics_present_queue = my_logical_device.get_hardware_queue();

        // last buffer in command pool is specifically for transfer.
        vk::CommandBuffer& transfer_cmd_buf = command_pool[swapchain.get_image_views().size()];
        {
            vk::Buffer vertices_staging{vk::BufferType::Staging, vk::BufferPurpose::TransferSource, my_logical_device, vk::hardware::MemoryResidency::CPU, vertices_bytes};
            vertices_staging.write(vertices.data(), vertices_bytes);
            {
                vk::CommandBufferRecording transfer_vertices = transfer_cmd_buf.record();
                transfer_vertices.buffer_copy_buffer(vertices_staging, buf, vertices_bytes);
            }

            vk::Fence wait_for_cpy{my_logical_device};
            wait_for_cpy.signal();
            vk::Submit do_staging_cpy{vk::CommandBuffers{transfer_cmd_buf}, vk::SemaphoreRefs{}, vk::WaitStages{}, vk::SemaphoreRefs{}};
            do_staging_cpy(graphics_present_queue, wait_for_cpy);
            wait_for_cpy.wait_for();

            // Then repeat the process for index buffer.
            vk::Buffer indices_staging{vk::BufferType::Staging, vk::BufferPurpose::TransferSource, my_logical_device, vk::hardware::MemoryResidency::CPU, indices_bytes};
            indices_staging.write(indices.data(), indices_bytes);
            transfer_cmd_buf.reset();
            {
                vk::CommandBufferRecording transfer_indices = transfer_cmd_buf.record();
                transfer_indices.buffer_copy_buffer(indices_staging, index_buf, indices_bytes);
            }

            wait_for_cpy.signal();
            do_staging_cpy(graphics_present_queue, wait_for_cpy);
            wait_for_cpy.wait_for();

            transfer_cmd_buf.reset();
            {
                vk::CommandBufferRecording transfer_image = transfer_cmd_buf.record();
                img.set_layout(transfer_image, vk::Image::Layout::TransferDestination);
                //transfer_image.transition_image_layout(img, vk::Image::Layout::TransferDestination);
                transfer_image.buffer_copy_image(img_buf, img);
                img.set_layout(transfer_image, vk::Image::Layout::ShaderResource);
                //transfer_image.transition_image_layout(img, vk::Image::Layout::ShaderResource);
            }

            wait_for_cpy.signal();
            do_staging_cpy(graphics_present_queue, wait_for_cpy);
            wait_for_cpy.wait_for();
        }
        
        vk::Semaphore image_available{my_logical_device};
        vk::Semaphore render_finished{my_logical_device};

        auto update_uniform_buffer = [&](std::uint32_t image_index)
        {
            static auto startTime = std::chrono::high_resolution_clock::now();

            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
            
            MVP mvp{};
            mvp.m = tz::model({0.0f, std::sin(time * 4.0f), -10.0f}, {0.0f, time * 2.0f, 0.0f}, {5.0f, 5.0f, 5.0f});
            mvp.v = tz::view({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f});
            mvp.p = tz::perspective(1.27f, swapchain.get_width() / swapchain.get_height(), 0.1f, 100.0f);
            mvp_bufs[image_index].write(&mvp, mvp_bytes);
        };


        auto regenerate = [&]()
        {
            swapchain_support = my_device.get_window_swapchain_support();
            vk::hardware::SwapchainSelectorPreferences my_prefs;
            my_prefs.format_pref = {vk::hardware::SwapchainFormatPreferences::Goldilocks, vk::hardware::SwapchainFormatPreferences::FlexibleGoldilocks, vk::hardware::SwapchainFormatPreferences::DontCare};
            my_prefs.present_mode_pref = {vk::hardware::SwapchainPresentModePreferences::PreferTripleBuffering, vk::hardware::SwapchainPresentModePreferences::DontCare};

            // VK starts malding if you try and kill the image views before the command buffers are done with them (fair enough tbh)
            my_logical_device.block_until_idle();
            // Old swapchain must die before new one can be created.
            // TODO: Have both around at once and grandfather the old one via oldSwapchain support.
            swapchain.~Swapchain();
            new (&swapchain) vk::Swapchain{my_logical_device, my_prefs};

            simple_colour_pass = {my_logical_device, builder};
            my_pipeline = {
                std::initializer_list<vk::pipeline::ShaderStage>{{vertex, vk::pipeline::ShaderType::Vertex}, {fragment, vk::pipeline::ShaderType::Fragment}},
                my_logical_device,
                vertex_input_state,
                vk::pipeline::InputAssembly{vk::pipeline::PrimitiveTopology::Triangles},
                vk::pipeline::ViewportState{swapchain},
                vk::pipeline::RasteriserState
                {
                    false,
                    false,
                    vk::pipeline::PolygonMode::Fill,
                    1.0f,
                    vk::pipeline::CullingStrategy::None
                },
                vk::pipeline::MultisampleState{},
                vk::pipeline::ColourBlendState{},
                vk::pipeline::DynamicState::None(),
                my_layout,
                simple_colour_pass
            };

            depth_img = {my_logical_device, static_cast<std::uint32_t>(swapchain.get_width()), static_cast<std::uint32_t>(swapchain.get_height()), vk::Image::Format::DepthFloat32, {vk::Image::Usage::DepthStencilAttachment}, vk::hardware::MemoryResidency::GPU};
            depth_img_view = {my_logical_device, depth_img};

            swapchain_buffers.clear();
            for(const vk::ImageView& swapchain_view : swapchain.get_image_views())
            {
                swapchain_buffers.emplace_back(simple_colour_pass, swapchain_view, depth_img_view, VkExtent2D{static_cast<std::uint32_t>(swapchain.get_width()), static_cast<std::uint32_t>(swapchain.get_height())});
            }

            command_pool.clear();
            command_pool.with(swapchain.get_image_views().size());
            for(std::size_t i = 0; i < swapchain.get_image_views().size(); i++)
            {
                vk::CommandBufferRecording render = command_pool[i].record();
                vk::RenderPassRun run{command_pool[i], simple_colour_pass, swapchain_buffers[i], swapchain.full_render_area(), VkClearValue{0.1f, 0.3f, 0.7f, 0.0f}};
                my_pipeline.bind(command_pool[i]);
                render.bind(buf);
                render.bind(index_buf);
                render.bind(descriptor_pool[i], my_layout);
                render.draw_indexed(indices.size());
            }
        };

        constexpr std::size_t num_frames_in_flight = 2;
        vk::FrameAdmin frame_admin{my_logical_device, num_frames_in_flight};
        frame_admin.set_regeneration_function(regenerate);
        tz::window().add_resize_callback([regenerate](int width, int height)
        {
            int w = width;
            int h = height;
            while(w == 0 || h == 0)
            {
                w = tz::window().get_width();
                h = tz::window().get_height();
                tz::Window::block_until_event_happens();
            }
            regenerate();
        });
        while(!tz::window().is_close_requested())
        {
            tz::window().update();
            update_uniform_buffer(frame_admin.get_image_index());
            frame_admin.render_frame(graphics_present_queue, swapchain, command_pool, vk::WaitStages{vk::WaitStage::ColourAttachmentOutput});
        }
        my_logical_device.block_until_idle();
    }
    tz::terminate();
    return 0;
}