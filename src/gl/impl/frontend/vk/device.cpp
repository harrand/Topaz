#if TZ_VULKAN
#include "core/tz.hpp"
#include "gl/impl/frontend/vk/device.hpp"
#include "gl/impl/backend/vk/tz_vulkan.hpp"
#include "gl/impl/backend/vk/hardware/device_filter.hpp"

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

    vk::Image::Format DeviceWindowBufferVulkan::get_format() const
    {
        if(vk::is_headless())
        {
            return std::get<vk::Image>(*this).get_format();
        }
        else
        {
            return std::get<vk::Swapchain>(*this).get_format();
        }
    }

    std::uint32_t DeviceWindowBufferVulkan::get_width() const
    {
        if(vk::is_headless())
        {
            return std::get<vk::Image>(*this).get_width();
        }
        else
        {
            return static_cast<std::uint32_t>(std::get<vk::Swapchain>(*this).get_width());
        }
    }

    unsigned int DeviceWindowBufferVulkan::get_height() const
    {
        if(vk::is_headless())
        {
            return std::get<vk::Image>(*this).get_height();
        }
        else
        {
            return static_cast<std::uint32_t>(std::get<vk::Swapchain>(*this).get_height());
        }
    }

    VkRect2D DeviceWindowBufferVulkan::full_render_area() const
    {
        if(vk::is_headless())
        {
            const vk::Image& as_image = std::get<vk::Image>(*this);
            return {.offset = {0, 0}, .extent = {as_image.get_width(), as_image.get_height()}};
        }
        else
        {
            return std::get<vk::Swapchain>(*this).full_render_area();
        }
    }

    DeviceWindowBufferVulkan& DeviceWindowBufferVulkan::operator=(vk::Swapchain&& rhs)
    {
        static_cast<DeviceWindowBufferVulkan::VariantType&>(*this) = std::move(rhs);
        return *this;
    }

    DeviceWindowBufferVulkan& DeviceWindowBufferVulkan::operator=(vk::Image&& rhs)
    {
        static_cast<DeviceWindowBufferVulkan::VariantType&>(*this) = std::move(rhs);
        return *this;
    }

    DeviceWindowBufferVulkan::operator vk::Swapchain&()
    {
        tz_assert(!vk::is_headless(), "DeviceWindowBufferVulkan is not a vk::Swapchain because application is headless");
        return std::get<vk::Swapchain>(*this);
    }

    DeviceWindowBufferVulkan::operator const vk::Swapchain&() const
    {
        tz_assert(!vk::is_headless(), "DeviceWindowBufferVulkan is not a vk::Swapchain because application is headless");
        return std::get<vk::Swapchain>(*this);
    }

    DeviceWindowBufferVulkan::operator vk::Image&()
    {
        tz_assert(vk::is_headless(), "DeviceWindowBufferVulkan is not a vk::Image because application isn't headless");
        return std::get<vk::Image>(*this);
    }

    DeviceWindowBufferVulkan::operator const vk::Image&() const
    {
        tz_assert(vk::is_headless(), "DeviceWindowBufferVulkan is not a vk::Image because application isn't headless");
        return std::get<vk::Image>(*this);
    }

    DeviceFunctionalityVulkan::DeviceFunctionalityVulkan():
    physical_device(vk::hardware::Device::null()),
    device(vk::LogicalDevice::null()),
    swapchain(),
    primitive_type(),
    renderer_resize_callbacks()
    {
        // Setup window resize support.
        if(!vk::is_headless())
        {
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
    }

    RenderPassVulkan DeviceFunctionalityVulkan::vk_create_render_pass(RenderPassBuilderVulkan builder) const
    {
        RenderPassDeviceInfoVulkan device_info;
        device_info.device = this;
        device_info.window_buffer = &this->swapchain;
        device_info.vk_device = &this->device;
        return {builder, device_info};
    }

    Renderer DeviceFunctionalityVulkan::create_renderer(RendererBuilder builder) const
    {
        RendererBuilderDeviceInfoVulkan device_info;
        device_info.creator_device = this;
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

    TextureFormat DeviceFunctionalityVulkan::get_window_format() const
    {
        switch(this->swapchain.get_format())
        {
            case vk::Image::Format::Rgba32Signed:
                return TextureFormat::Rgba32Signed;
            break;
            case vk::Image::Format::Rgba32Unsigned:
                return TextureFormat::Rgba32Unsigned;
            break;
            case vk::Image::Format::Rgba32sRGB:
                return TextureFormat::Rgba32sRGB;
            break;
            case vk::Image::Format::DepthFloat32:
                return TextureFormat::DepthFloat32;
            break;
            case vk::Image::Format::Bgra32UnsignedNorm:
                return TextureFormat::Bgra32UnsignedNorm;
            break;
            default:
                tz_error("Unrecognised underlying window format. Behaviour of program is undefined.");
                return {};
            break;
        }
    }

    DeviceVulkan::DeviceVulkan(DeviceBuilderVulkan builder):
    DeviceFunctionalityVulkan()
    {
        vk::ExtensionList extensions{"VK_KHR_maintenance1"};
        if(!vk::is_headless())
        {
            extensions.add("VK_KHR_swapchain");
        }
        vk::hardware::DeviceList all_devices = vk::hardware::get_all_devices();
        // Ideally we want a queue which supports all 3 of graphics, transfer and present.
        vk::hardware::QueueFamilyTypeField type_requirements{{vk::hardware::QueueFamilyType::Graphics, vk::hardware::QueueFamilyType::Transfer}};
        // We require swapchain support too.
        vk::hardware::DeviceFilterList filters;
        filters.emplace<vk::hardware::DeviceQueueFamilyFilter>(type_requirements);
        filters.emplace<vk::hardware::DeviceExtensionSupportFilter>(extensions);
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
        this->device = {maybe_chosen_queue_family.value(), extensions};
        if(vk::is_headless())
        {
            this->swapchain = vk::Image{this->device, 800, 600, vk::Image::Format::Rgba32sRGB, vk::Image::UsageField{vk::Image::Usage::ColourAttachment, vk::Image::Usage::TransferSource}, vk::hardware::MemoryResidency::GPU};
        }
        else
        {
            vk::hardware::SwapchainSelectorPreferences my_prefs;
            my_prefs.format_pref = {vk::hardware::SwapchainFormatPreferences::Goldilocks, vk::hardware::SwapchainFormatPreferences::FlexibleGoldilocks, vk::hardware::SwapchainFormatPreferences::DontCare};
            my_prefs.present_mode_pref = {vk::hardware::SwapchainPresentModePreferences::PreferTripleBuffering, vk::hardware::SwapchainPresentModePreferences::DontCare};
            
            this->swapchain = vk::Swapchain{this->device, my_prefs};
        }
        
        this->primitive_type = builder.vk_get_primitive_topology();
    }

    void DeviceFunctionalityVulkan::on_window_resize()
    {
        this->device.block_until_idle();
        if(!vk::is_headless())
        {
            // First update swapchain.
            vk::hardware::SwapchainSelectorPreferences my_prefs;
            my_prefs.format_pref = {vk::hardware::SwapchainFormatPreferences::Goldilocks, vk::hardware::SwapchainFormatPreferences::FlexibleGoldilocks, vk::hardware::SwapchainFormatPreferences::DontCare};
            my_prefs.present_mode_pref = {vk::hardware::SwapchainPresentModePreferences::PreferTripleBuffering, vk::hardware::SwapchainPresentModePreferences::DontCare};

            vk::Swapchain& real_swapchain = std::get<vk::Swapchain>(this->swapchain);
            real_swapchain.~Swapchain();
            new (&real_swapchain) vk::Swapchain(this->device, my_prefs);
        }
        // Then notify all renderers which care. Do it in reverse as renderers that rely on other renderers will have been created before.
        for(auto i = this->renderer_resize_callbacks.rbegin(); i != this->renderer_resize_callbacks.rend(); i++)
        {
            const DeviceWindowResizeCallback& callback = *i;
            if(callback != nullptr)
            {
                callback();
            }
        }
    }
}

#endif // TZ_VULKAN