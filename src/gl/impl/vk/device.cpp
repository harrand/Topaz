#if TZ_VULKAN
#include "core/tz.hpp"
#include "gl/impl/vk/device.hpp"
#include "gl/vk/hardware/device_filter.hpp"

namespace tz::gl
{
    DeviceBuilderVulkan::DeviceBuilderVulkan():
    primitive_type(GraphicsPrimitiveType::Triangles)
    {

    }

    void DeviceBuilderVulkan::set_primitive_type(GraphicsPrimitiveType type)
    {
        this->primitive_type = type;
    }

    GraphicsPrimitiveType DeviceBuilderVulkan::get_primitive_type() const
    {
        return this->primitive_type;
    }

    vk::pipeline::PrimitiveTopology DeviceBuilderVulkan::vk_get_primitive_topology() const
    {
        switch(this->primitive_type)
        {
            case GraphicsPrimitiveType::Triangles:
                return vk::pipeline::PrimitiveTopology::Triangles;
            break;
            default:
                tz_error("GraphicsPrimitiveType not supported");
                return vk::pipeline::PrimitiveTopology::Triangles;
            break;
        }
    }

    DeviceFunctionalityVulkan::DeviceFunctionalityVulkan():
    physical_device(vk::hardware::Device::null()),
    device(vk::LogicalDevice::null()),
    swapchain(vk::Swapchain::null()),
    primitive_type(),
    renderer_resize_callbacks()
    {
        // Setup window resize support.
        tz::window().add_resize_callback([this](int width, int height)
        {
            int w = width;
            int h = height;
            while(w == 0 || h == 0)
            {
                w = tz::window().get_width();
                h = tz::window().get_height();
                tz::Window::block_until_event_happens();
            }
            this->on_window_resize();
        });
    }

    RenderPass DeviceFunctionalityVulkan::create_render_pass(RenderPassBuilder builder) const
    {
        builder.vk_finalise(this->swapchain.get_format());
        RenderPassBuilderDeviceInfoVulkan device_info;
        device_info.device = &this->device;
        device_info.device_swapchain = &this->swapchain;
        return {builder, device_info};
    }

    Renderer DeviceFunctionalityVulkan::create_renderer(RendererBuilder builder) const
    {
        RendererBuilderDeviceInfoVulkan device_info;
        device_info.device = &this->device;
        device_info.primitive_type = this->primitive_type;
        device_info.device_swapchain = &this->swapchain;
        device_info.on_resize = &this->renderer_resize_callbacks.emplace_back(nullptr);
        return {builder, device_info};
    }

    Shader DeviceFunctionalityVulkan::create_shader(ShaderBuilder builder) const
    {
        return {this->device, builder};
    }

    DeviceVulkan::DeviceVulkan(DeviceBuilderVulkan builder):
    DeviceFunctionalityVulkan()
    {
        vk::hardware::DeviceList all_devices = vk::hardware::get_all_devices();
        // Ideally we want a queue which supports all 3 of graphics, transfer and present.
        vk::hardware::QueueFamilyTypeField type_requirements{{vk::hardware::QueueFamilyType::Graphics, vk::hardware::QueueFamilyType::Transfer}};
        // We require swapchain support too.
        vk::hardware::DeviceFilterList filters;
        filters.emplace<vk::hardware::DeviceQueueFamilyFilter>(type_requirements);
        filters.emplace<vk::hardware::DeviceExtensionSupportFilter>(std::initializer_list<vk::VulkanExtension>{"VK_KHR_swapchain", "VK_KHR_maintenance1"});
        // Filter out all devices which don't match our description.
        filters.filter_all(all_devices);
        tz_assert(!all_devices.empty(), "Physical Device requirements are too stringent for this machine. Topaz Vulkan does not support your hardware.");
        // TODO: Rate all suitable physical devices and choose the best one.
        this->physical_device = all_devices.front();
        // Choose the queue family which is most suitable to our needs.
        std::optional<vk::hardware::DeviceQueueFamily> maybe_chosen_queue_family;
        for(vk::hardware::DeviceQueueFamily queue_family : this->physical_device.get_queue_families())
        {
            if(queue_family.types_supported.contains(type_requirements))
            {
                maybe_chosen_queue_family = queue_family;
            }
        }
        tz_assert(maybe_chosen_queue_family.has_value(), "Valid device found which supports present, graphics and transfer, but not a single queue that can do both. Topaz Vulkan does not support your hardware.");
        this->device = {maybe_chosen_queue_family.value(), {"VK_KHR_swapchain"}};
        vk::hardware::SwapchainSelectorPreferences my_prefs;
        my_prefs.format_pref = {vk::hardware::SwapchainFormatPreferences::Goldilocks, vk::hardware::SwapchainFormatPreferences::FlexibleGoldilocks, vk::hardware::SwapchainFormatPreferences::DontCare};
        my_prefs.present_mode_pref = {vk::hardware::SwapchainPresentModePreferences::PreferTripleBuffering, vk::hardware::SwapchainPresentModePreferences::DontCare};
        
        this->swapchain = {this->device, my_prefs};
        this->primitive_type = builder.vk_get_primitive_topology();
    }

    void DeviceFunctionalityVulkan::on_window_resize()
    {
        // First update swapchain.
        vk::hardware::SwapchainSelectorPreferences my_prefs;
        my_prefs.format_pref = {vk::hardware::SwapchainFormatPreferences::Goldilocks, vk::hardware::SwapchainFormatPreferences::FlexibleGoldilocks, vk::hardware::SwapchainFormatPreferences::DontCare};
        my_prefs.present_mode_pref = {vk::hardware::SwapchainPresentModePreferences::PreferTripleBuffering, vk::hardware::SwapchainPresentModePreferences::DontCare};

        this->device.block_until_idle();
        this->swapchain.~Swapchain();
        new (&this->swapchain) vk::Swapchain(this->device, my_prefs);
        // Then notify all renderers which care.
        for(const DeviceWindowResizeCallback& callback : this->renderer_resize_callbacks)
        {
            if(callback != nullptr)
            {
                callback();
            }
        }
    }
}

#endif // TZ_VULKAN