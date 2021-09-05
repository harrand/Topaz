#include "gl/impl/frontend/common/processor.hpp"

#include "core/assert.hpp"
namespace tz::gl
{
    ResourceHandle ProcessorBuilderBase::add_resource(const IResource& resource)
    {
        std::size_t total_resource_size = this->buffer_resources.size() + this->texture_resources.size();
        switch(resource.get_type())
        {
            case ResourceType::Buffer:
                this->buffer_resources.push_back(&resource);
            break;
            case ResourceType::Texture:
                this->texture_resources.push_back(&resource);
            break;
            default:
                tz_error("Unsupported ResourceType");
                return {static_cast<tz::HandleValue>(0)};
            break;
        }
        return static_cast<tz::HandleValue>(total_resource_size);
    }

    const IResource* ProcessorBuilderBase::get_resource(ResourceHandle handle) const
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

    void ProcessorBuilderBase::set_shader(const Shader& shader)
    {
        this->shader = &shader;
    }

    const Shader& ProcessorBuilderBase::get_shader() const
    {
        tz_assert(this->shader != nullptr, "No shader was set previously so cannot retrieve the current shader");
        return *this->shader;
    }
}