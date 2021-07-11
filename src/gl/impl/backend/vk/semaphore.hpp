#ifndef TOPAZ_GL_VK_SEMAPHORE_HPP
#define TOPAZ_GL_VK_SEMAPHORE_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk/logical_device.hpp"

namespace tz::gl::vk
{
    class Semaphore
    {
    public:
        Semaphore(const LogicalDevice& device);
        Semaphore(const Semaphore& copy) = delete;
        Semaphore(Semaphore&& move);
        ~Semaphore();

        Semaphore& operator=(const Semaphore& rhs) = delete;
        Semaphore& operator=(Semaphore&& rhs);

        VkSemaphore native() const;
    private:
        VkSemaphore sem;
        const LogicalDevice* device;
    };

    using SemaphoreRefs = std::initializer_list<std::reference_wrapper<const Semaphore>>;
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_SEMAPHORE_HPP