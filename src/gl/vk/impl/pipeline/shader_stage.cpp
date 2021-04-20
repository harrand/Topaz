#if TZ_VULKAN
#include "gl/vk/impl/pipeline/shader_stage.hpp"
#include "core/assert.hpp"

namespace tz::gl::vk::pipeline
{
    ShaderStage::ShaderStage(const ShaderModule& module, ShaderType type):
    create(),
    type(type)
    {
        this->create.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        switch(type)
        {
            case ShaderType::Vertex:
                this->create.stage = VK_SHADER_STAGE_VERTEX_BIT;
            break;
            case ShaderType::Fragment:
                this->create.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            break;
            default:
                tz_error("No ShaderStage support implemented for the given ShaderType. You probably forgot to implement it");
            break;
        }
        this->create.module = module.native();
        this->create.pName = "main";
    }

    ShaderType ShaderStage::get_type() const
    {
        return this->type;
    }

    VkPipelineShaderStageCreateInfo ShaderStage::native() const
    {
        return this->create;
    }
}

#endif // TZ_VULKAN