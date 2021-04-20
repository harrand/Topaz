#ifndef TOPAZ_GL_VK_PIPELINE_SHADER_STAGE_HPP
#define TOPAZ_GL_VK_PIPELINE_SHADER_STAGE_HPP
#if TZ_VULKAN
#include "gl/vk/impl/pipeline/shader_module.hpp"

namespace tz::gl::vk::pipeline
{
    enum class ShaderType
    {
        Vertex,
        Fragment
    };

    class ShaderStage
    {
    public:
        ShaderStage(const ShaderModule& module, ShaderType type);
        ShaderType get_type() const;
        VkPipelineShaderStageCreateInfo native() const;
    private:
        VkPipelineShaderStageCreateInfo create;
        ShaderType type;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_PIPELINE_SHADER_STAGE_HPP