#if TZ_VULKAN
#include "gl/impl/backend/vk/pipeline/shader_module.hpp"
#include "core/assert.hpp"
#include <cstdint>

namespace tz::gl::vk
{
    ShaderModule::ShaderModule(const LogicalDevice& device, const std::span<const char>& spirv):
    device(&device),
    shader_module(VK_NULL_HANDLE)
    {
        VkShaderModuleCreateInfo create{};
        create.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create.codeSize = spirv.size();
        create.pCode = reinterpret_cast<const std::uint32_t*>(spirv.data());

        auto res = vkCreateShaderModule(this->device->native(), &create, nullptr, &this->shader_module);
        tz_assert(res == VK_SUCCESS, "tz::gl::vk::ShaderModule::ShaderModule(...): Failed to create shader module");
    }

    ShaderModule::ShaderModule(ShaderModule&& move):
    device(nullptr),
    shader_module(VK_NULL_HANDLE)
    {
        *this = std::move(move);
    }

    ShaderModule::~ShaderModule()
    {
        if(this->shader_module != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(this->device->native(), this->shader_module, nullptr);
            this->shader_module = VK_NULL_HANDLE;
        }
    }

    ShaderModule& ShaderModule::operator=(ShaderModule&& rhs)
    {
        std::swap(this->device, rhs.device);
        std::swap(this->shader_module, rhs.shader_module);
        return *this;
    }

    VkShaderModule ShaderModule::native() const
    {
        return this->shader_module;
    }
}


#endif // TZ_VULKAN