#if !TZ_VULKAN
    static_assert(false, "Cannot build vk_device_demo with TZ_VULKAN disabled.");
#endif

#include "gl/vk/setup/vulkan_instance.hpp"
#include "gl/vk/hardware/device.hpp"
#include "gl/vk/hardware/device_filter.hpp"
#include <cstdio>

const char* get_device_type_name(VkPhysicalDeviceType t)
{
    switch(t)
    {
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            return "Other Device Type";
        break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            return "Integrated GPU";
        break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            return "Discrete GPU";
        break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            return "Virtual GPU";
        break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            return "CPU";
        break;
        default:
            return "Unknown Device Type";
        break;
    }
}

void print_list(const char* label, const tz::gl::vk::hardware::DeviceList& devices)
{
    std::printf("\"%s\" (size %zu): { ", label, devices.length());
    for(const tz::gl::vk::hardware::Device& dev : devices)
    {
        auto props = dev.get_properties();
        std::printf("[\"%s\" (%s)] ", props.deviceName, get_device_type_name(props.deviceType));
    }
    std::printf("}\n");
}

int main()
{
    constexpr tz::EngineInfo eng_info = tz::info();
    tz::GameInfo vk_init_demo{"vk_device_demo", eng_info.version, eng_info};
    namespace tzvk = tz::gl::vk;
    tzvk::VulkanApplicationInfo vk_info{vk_init_demo};

    tzvk::VulkanInstance vk_inst{vk_info};
    tzvk::hardware::Device::List all_devices = tzvk::hardware::get_all_devices(vk_inst());

    tzvk::hardware::Device::List devices = tzvk::hardware::get_all_devices(vk_inst());
    print_list("Unfiltered", devices);

    tzvk::hardware::quick_filters::remove_non_gpus(devices);
    print_list("Non-GPUs Removed", devices);
    devices = all_devices;

    tzvk::hardware::quick_filters::preserve_only_integrated_gpus(devices);
    print_list("Only integrated GPUs", devices);
    devices = all_devices;

    tzvk::hardware::quick_filters::preserve_only_supporting_queues<VK_QUEUE_GRAPHICS_BIT>(devices);
    print_list("Only those with a graphics queue", devices);
    devices = all_devices;

    tzvk::hardware::quick_filters::preserve_only_supporting_queues<VK_QUEUE_COMPUTE_BIT>(devices);
    print_list("Only those with a compute queue", devices);
    devices = all_devices;

    tzvk::hardware::quick_filters::preserve_only_supporting_queues<VK_QUEUE_TRANSFER_BIT>(devices);
    print_list("Only those with a transfer queue", devices);
    devices = all_devices;

    tzvk::hardware::quick_filters::preserve_only_supporting_queues<VK_QUEUE_SPARSE_BINDING_BIT>(devices);
    print_list("Only those with a sparse binding queue", devices);
    devices = all_devices;

    tzvk::hardware::quick_filters::preserve_only_supporting_queues<VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_SPARSE_BINDING_BIT>(devices);
    print_list("Only those with at least one queue supporting one of each: graphics, compute, transfer, sparse binding", devices);
    devices = all_devices;


    return 0;
}