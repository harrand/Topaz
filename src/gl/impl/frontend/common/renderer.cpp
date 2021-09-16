#include "gl/impl/frontend/common/renderer.hpp"

namespace tz::gl
{
    RendererInputHandle RendererBuilderBase::add_input(const IRendererInput& input)
    {
        auto sz = this->inputs.size();
        this->inputs.push_back(&input);
        return static_cast<tz::HandleValue>(sz);
    }

    const IRendererInput* RendererBuilderBase::get_input(RendererInputHandle handle) const
    {
        std::size_t input_id = static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
        tz_assert(input_id < this->inputs.size(), "Handle %zu is invalid for this RendererInput. Perhaps this input belongs to another Renderer?", input_id);
        return this->inputs[input_id];
    }

    void RendererBuilderBase::set_output(IRendererOutput& output)
    {
        this->output = &output;
    }

    const IRendererOutput* RendererBuilderBase::get_output() const
    {
        return this->output;
    }

    IRendererOutput* RendererBuilderBase::get_output()
    {
        return this->output;
    }

    ResourceHandle RendererBuilderBase::add_resource(const IResource& resource)
    {
        std::size_t total_resource_size = this->buffer_resources.size() + this->texture_resources.size();
        switch(resource.get_type())
        {
            case ResourceType::Buffer:
                this->buffer_resources.push_back(&resource);
                return {static_cast<tz::HandleValue>(total_resource_size)};
            break;
            case ResourceType::Texture:
                this->texture_resources.push_back(&resource);
                return {static_cast<tz::HandleValue>(total_resource_size)};
            break;
            default:
                tz_error("Unexpected resource type. Support for this resource type is not yet implemented (Vulkan)");
                return {static_cast<tz::HandleValue>(0)};
            break;
        }
    }

    const IResource* RendererBuilderBase::get_resource(ResourceHandle handle) const
    {
        auto handle_value = static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
        if(handle_value >= this->buffer_resources.size())
        {
            // Handle value doesn't fit within buffer resources, must be a texture resource or invalid.
            if(handle_value < this->buffer_resources.size() + this->texture_resources.size())
            {
                // Is within range, we assume it's a valid texture resource
                return this->texture_resources[handle_value - this->buffer_resources.size()];
            }
            else
            {
                // Invalid, probably someone else's ResourceHsndle
                return nullptr;
            }
        }
        // Is within range, we assume it's a valid buffer resource
        return this->buffer_resources[handle_value];
    }

    void RendererBuilderBase::set_culling_strategy(RendererCullingStrategy culling_strategy)
    {
        this->culling_strategy = culling_strategy;
    }

    RendererCullingStrategy RendererBuilderBase::get_culling_strategy() const
    {
        return this->culling_strategy;
    }

    void RendererBuilderBase::set_shader(const Shader& shader)
    {
        this->shader = &shader;
    }

    const Shader& RendererBuilderBase::get_shader() const
    {
        tz_assert(this->shader != nullptr, "No shader yet");
        return *this->shader;
    }
}