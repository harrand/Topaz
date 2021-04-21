#if !TZ_VULKAN
    static_assert(false, "Cannot build vk_device_demo with TZ_VULKAN disabled.");
#endif

#include "core/tz.hpp"
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
    constexpr tz::GameInfo vk_device_demo{"vk_device_demo", eng_info.version, eng_info};
    tz::initialise(vk_device_demo, tz::ApplicationType::Headless);
    {
        namespace tzvk = tz::gl::vk;
        tzvk::hardware::Device::List all_devices = tzvk::hardware::get_all_devices();

        tzvk::hardware::Device::List devices = tzvk::hardware::get_all_devices();
        print_list("Unfiltered", devices);

        tzvk::hardware::quick_filters::remove_non_gpus(devices);
        print_list("Non-GPUs Removed", devices);
        devices = all_devices;

        tzvk::hardware::quick_filters::preserve_only_integrated_gpus(devices);
        print_list("Only integrated GPUs", devices);
        devices = all_devices;

        tzvk::hardware::quick_filters::preserve_only_with_appropriate_queue(devices, {{tzvk::hardware::QueueFamilyType::Graphics}});
        print_list("Only those with a graphics queue", devices);
        devices = all_devices;

        tzvk::hardware::quick_filters::preserve_only_with_appropriate_queue(devices, {{tzvk::hardware::QueueFamilyType::Compute}});
        print_list("Only those with a compute queue", devices);
        devices = all_devices;

        tzvk::hardware::quick_filters::preserve_only_with_appropriate_queue(devices, {{tzvk::hardware::QueueFamilyType::Transfer}});
        print_list("Only those with a transfer queue", devices);
        devices = all_devices;

        tzvk::hardware::quick_filters::preserve_only_with_appropriate_queue(devices, {{tzvk::hardware::QueueFamilyType::SparseBinding}});
        print_list("Only those with a sparse binding queue", devices);
        devices = all_devices;

        tzvk::hardware::QueueFamilyTypeField all{{tzvk::hardware::QueueFamilyType::Graphics, tzvk::hardware::QueueFamilyType::Compute, tzvk::hardware::QueueFamilyType::Transfer, tzvk::hardware::QueueFamilyType::SparseBinding}};
        tzvk::hardware::quick_filters::preserve_only_with_appropriate_queue(devices, all);
        print_list("Only those with at least one queue supporting one of each: graphics, compute, transfer, sparse binding", devices);
        devices = all_devices;
    }
    tz::terminate();
    return 0;
}