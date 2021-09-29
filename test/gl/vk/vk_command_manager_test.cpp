#include "core/tz.hpp"
#include "gl/impl/backend/vk/extra/command_manager.hpp"
#include "gl/impl/backend/vk/hardware/device_filter.hpp"

tz::gl::vk::LogicalDevice make_device()
{
    using namespace tz::gl;
    vk::hardware::DeviceList valid_devices = tz::gl::vk::hardware::get_all_devices();
    vk::hardware::QueueFamilyTypeField type_requirements{{vk::hardware::QueueFamilyType::Graphics}};
    // Let's grab a device which can do graphics and present images.
    // It also must support the swapchain device extension.
    {
        namespace hw = vk::hardware;
        hw::DeviceFilterList filters;
        filters.emplace<hw::DeviceQueueFamilyFilter>(type_requirements);
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
    return {my_qfam};
}

int main()
{
    tz::initialise(tz::GameInfo{"vk_command_manager_test", tz::Version{1, 0, 0}}, tz::ApplicationType::Headless);
    {
        tz::gl::vk::LogicalDevice dev = make_device();
        tz::gl::vk::extra::CommandManagerInfo info
        {
            .device = &dev,
            .pool_size_buffers = 2
        };
        tz::gl::vk::extra::CommandManager mgr{info};
        {
            tz_assert(mgr.pool_count() == 0, "Empty CommandManager had pool count == %zu which is != 0", mgr.pool_count());
            tz_assert(mgr.buffer_count() == 0, "Empty CommandManager had buffer count == %zu which is != 0", mgr.buffer_count());
            tz::gl::vk::extra::CommandBufferHandle b1 = mgr.add_buffer();
            tz_assert(mgr.pool_count() == 1, "CommandManager had pool count == %zu which is != 0", mgr.pool_count());
            tz_assert(mgr.buffer_count() == 1, "CommandManager had buffer count == %zu which is != 1", mgr.buffer_count());
            // 2 buffers per pool, so even with 2 buffers, pool count should still be 1.
            tz::gl::vk::extra::CommandBufferHandle b2 = mgr.add_buffer();
            tz_assert(mgr.pool_count() == 1, "CommandManager had pool count == %zu which is != 1", mgr.pool_count());
            tz_assert(mgr.buffer_count() == 2, "CommandManager had buffer count == %zu which is != 2", mgr.buffer_count());
            tz_assert(b1 != b2, "CommandManager: Different CommandBufferHandles equate to true. That ain't right");
            tz_assert(mgr[b1] != mgr[b2], "CommandManager: Different CommandBuffers through different handles equated equated to true. That ain't right");

            // a third buffer *should* indeed create a second pool.
            tz::gl::vk::extra::CommandBufferHandle b3 = mgr.add_buffer();
            tz_assert(mgr.pool_count() == 2, "CommandManager had pool count == %zu which is != 2", mgr.pool_count());
            tz_assert(mgr.buffer_count() == 3, "CommandManager had buffer count == %zu which is != 3", mgr.buffer_count());
            tz_assert(b3 != b2 && b3 != b1, "CommandManager: Different CommandBufferHandles equate to true. That ain't right");
            tz_assert(mgr[b3] != mgr[b2] && mgr[b3] != mgr[b1], "CommandManager: Different CommandBuffers through different handles equated equated to true. That ain't right");

            // What if we erase one?
            mgr.erase(b3);
            // Pool count should be unchanged, but buffer count should be reduced by one.
            tz_assert(mgr.pool_count() == 2, "CommandManager had pool count == %zu which is != 2", mgr.pool_count());
            tz_assert(mgr.buffer_count() == 2, "CommandManager had buffer count == %zu which is != 2.  This was after an erase, so perhaps that's not affecting buffer_count() correctly?", mgr.buffer_count());

            // Finally, let's try using CommandManager::at(handle)
            tz_assert(mgr.at(b1) != nullptr, "CommandManager::at(valid handle) returned nullptr");
            tz_assert(mgr.at(b2) != nullptr, "CommandManager::at(valid handle) returned nullptr");
            tz_assert(mgr.at(b3) == nullptr, "CommandManager::at(freed handle) did not return nullptr");
            tz::gl::vk::extra::CommandBufferHandle nonsense_handle = static_cast<tz::HandleValue>(999999);
            tz_assert(mgr.at(nonsense_handle) == nullptr, "CommandManager::at(out of range handle) did not return nullptr");
        }
        mgr.clear();
        tz_assert(mgr.pool_count() == 0, "Empty CommandManager had pool count == %zu which is != 0", mgr.pool_count());
        tz_assert(mgr.buffer_count() == 0, "Empty CommandManager had buffer count == %zu which is != 0", mgr.buffer_count());
    }
    tz::terminate();
}