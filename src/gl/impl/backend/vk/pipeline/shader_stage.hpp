#ifndef TOPAZ_GL_VK_PIPELINE_SHADER_STAGE_HPP
#define TOPAZ_GL_VK_PIPELINE_SHADER_STAGE_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk/pipeline/shader_module.hpp"
#include "core/containers/enum_field.hpp"

namespace tz::gl::vk::pipeline
{
    enum class ShaderType
    {
        Vertex,
        Fragment
    };

    class ShaderTypeField : public tz::EnumField<ShaderType>
    {
    public:
        ShaderTypeField() = default;
        ShaderTypeField(std::initializer_list<ShaderType> types);
        static ShaderTypeField All();
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