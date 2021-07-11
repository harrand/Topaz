#if TZ_VULKAN
#include "gl/impl/backend/vk/semaphore.hpp"

namespace tz::gl::vk
{
    Semaphore::Semaphore(const LogicalDevice& device):
    sem(VK_NULL_HANDLE),
    device(&device)
    {
        VkSemaphoreCreateInfo create{};
        create.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        auto res = vkCreateSemaphore(this->device->native(), &create, nullptr, &this->sem);
        tz_assert(res == VK_SUCCESS, "Failed to create semaphore");
    }

    Semaphore::Semaphore(Semaphore&& move):
    sem(VK_NULL_HANDLE),
    device(nullptr)
    {
        *this = std::move(move);
    }

    Semaphore::~Semaphore()
    {
        if(this->sem != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(this->device->native(), this->sem, nullptr);
            this->sem = VK_NULL_HANDLE;
        }
    }

    Semaphore& Semaphore::operator=(Semaphore&& rhs)
    {
        std::swap(this->sem, rhs.sem);
        std::swap(this->device, rhs.device);
        return *this;
    }

    VkSemaphore Semaphore::native() const
    {
        return this->sem;
    }

}

#endif // TZ_VULKAN