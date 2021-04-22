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

int main()
{
    constexpr tz::EngineInfo eng_info = tz::info();
    constexpr tz::GameInfo vk_triangle_demo{"vk_triangle_demo", eng_info.version, eng_info};
    tz::initialise(vk_triangle_demo);
    {
        using namespace tz::gl;
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

        vk::RenderPassBuilder builder;
        vk::Attachment col
        {
            swapchain.get_format(), /* AKA vk::Image::Format::Rgba32sRGB for my machine */
            vk::Attachment::LoadOperation::Clear,
            vk::Attachment::StoreOperation::Store,
            vk::Image::Layout::Undefined,
            vk::Image::Layout::Present
        };

        builder.with(vk::Attachments{col});
        vk::RenderPass simple_colour_pass{my_logical_device, builder};

        vk::pipeline::Layout my_layout{my_logical_device};

        vk::ShaderModule vertex{my_logical_device, vk::read_external_shader(".\\demo\\gl\\vk\\basic.vertex.glsl").value()};
        vk::ShaderModule fragment{my_logical_device, vk::read_external_shader(".\\demo\\gl\\vk\\basic.fragment.glsl").value()};

        vk::GraphicsPipeline my_pipeline
        {
            std::initializer_list<vk::pipeline::ShaderStage>{{vertex, vk::pipeline::ShaderType::Vertex}, {fragment, vk::pipeline::ShaderType::Fragment}},
            my_logical_device,
            vk::pipeline::VertexInputState{},
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
            my_layout,
            simple_colour_pass
        };

        std::vector<vk::Framebuffer> swapchain_buffers;
        for(const vk::ImageView& swapchain_view : swapchain.get_image_views())
        {
            swapchain_buffers.emplace_back(simple_colour_pass, swapchain_view, VkExtent2D{static_cast<std::uint32_t>(swapchain.get_width()), static_cast<std::uint32_t>(swapchain.get_height())});
        }

        vk::CommandPool command_pool(my_logical_device, my_qfam);
        command_pool.with(swapchain.get_image_views().size());
        for(std::size_t i = 0; i < swapchain.get_image_views().size(); i++)
        {
            command_pool[i].begin_recording();
            {
                vk::RenderPassRun run{command_pool[i], simple_colour_pass, swapchain_buffers[i], swapchain.full_render_area(), VkClearValue{0.1f, 0.3f, 0.7f, 0.0f}};
                my_pipeline.bind(command_pool[i]);
                command_pool[i].draw(3, 1);
            };
            command_pool[i].end_recording();
        }
        
        vk::Semaphore image_available{my_logical_device};
        vk::Semaphore render_finished{my_logical_device};

        vk::hardware::Queue graphics_present_queue = my_logical_device.get_hardware_queue();

        constexpr std::size_t num_frames_in_flight = 2;
        vk::FrameAdmin frame_admin{my_logical_device, num_frames_in_flight};
        while(!tz::window().is_close_requested())
        {
            tz::window().update();
            frame_admin.render_frame(graphics_present_queue, swapchain, command_pool, vk::WaitStages{vk::WaitStage::ColourAttachmentOutput});
        }
        my_logical_device.block_until_idle();
    }
    tz::terminate();
    return 0;
}