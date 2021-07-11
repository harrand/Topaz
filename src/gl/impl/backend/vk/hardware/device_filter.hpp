#ifndef TOPAZ_GL_VK_HARDWARE_DEVICE_FILTER_HPP
#define TOPAZ_GL_VK_HARDWARE_DEVICE_FILTER_HPP
#if TZ_VULKAN
#include "core/containers/polymorphic_list.hpp"
#include "gl/impl/backend/vk/setup/extension_list.hpp"
#include "gl/impl/backend/vk/hardware/device.hpp"

namespace tz::gl::vk::hardware
{
    class IDeviceFilter
    {
    public:
        virtual ~IDeviceFilter() = default;
        virtual bool satisfies(const hardware::Device& device) const = 0;
    };

    class DeviceFilterList : public tz::PolymorphicList<IDeviceFilter>
    {
    public:
        DeviceFilterList() = default;  
        bool satisfies(const hardware::Device& device) const;
        void filter_all(hardware::DeviceList& devices) const;
    };

    class DeviceQueueFamilyFilter : public IDeviceFilter
    {
    public:
        DeviceQueueFamilyFilter(QueueFamilyTypeField types);
        virtual bool satisfies(const hardware::Device& device) const override;
    private:
        QueueFamilyTypeField types;
    };

    class DeviceExtensionSupportFilter : public IDeviceFilter
    {
    public:
        DeviceExtensionSupportFilter(ExtensionList extension_names);
        virtual bool satisfies(const hardware::Device& device) const override;
    private:
        ExtensionList required_extensions;
    };

    template<VkPhysicalDeviceType Type>
    class DeviceTypeFilter : public IDeviceFilter
    {
    public:
        constexpr DeviceTypeFilter() = default;
        virtual bool satisfies(const hardware::Device& device) const override;
    };

    using DeviceIntegratedGPUFilter = DeviceTypeFilter<VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU>;
    using DeviceDiscreteGPUFilter = DeviceTypeFilter<VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU>;
    using DeviceVirtualGPUFilter = DeviceTypeFilter<VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU>;
    using DeviceCPUFilter = DeviceTypeFilter<VK_PHYSICAL_DEVICE_TYPE_CPU>;

    class DeviceAnyGPUFilter : public IDeviceFilter
    {
    public:
        constexpr DeviceAnyGPUFilter() = default;
        virtual bool satisfies(const hardware::Device& device) const override;
    };

    namespace quick_filters
    {
        void remove_non_gpus(hardware::DeviceList& list);
        void preserve_only_discrete_gpus(hardware::DeviceList& list);
        void preserve_only_integrated_gpus(hardware::DeviceList& list);
        void preserve_only_with_appropriate_queue(hardware::DeviceList& list, QueueFamilyTypeField field);
    }
}

#include "gl/impl/backend/vk/hardware/device_filter.inl"
#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_HARDWARE_DEVICE_FILTER_HPP