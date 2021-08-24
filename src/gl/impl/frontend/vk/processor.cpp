#if TZ_VULKAN
#include "gl/impl/frontend/vk/processor.hpp"

namespace tz::gl
{
    ResourceHandle ProcessorBuilderVulkan::add_resource(const IResource& resource)
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

    const IResource* ProcessorBuilderVulkan::get_resource(ResourceHandle handle) const
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

    void ProcessorBuilderVulkan::set_shader(const Shader& shader)
    {
        this->shader = &shader;
    }

    const Shader& ProcessorBuilderVulkan::get_shader() const
    {
        tz_assert(this->shader != nullptr, "No shader was set previously so cannot retrieve the current shader");
        return *this->shader;
    }

    vk::DescriptorSetLayout ProcessorBuilderVulkan::vk_get_descriptor_set_layout(const vk::LogicalDevice& device) const
    {
        vk::LayoutBuilder layout_builder;
        for(std::size_t i = 0; i < this->buffer_resources.size(); i++)
        {
            layout_builder.add(vk::DescriptorType::UniformBuffer, {vk::pipeline::ShaderType::Compute});
        }
        for(std::size_t i = this->buffer_resources.size(); i < this->buffer_resources.size() + this->texture_resources.size(); i++)
        {
            layout_builder.add(vk::DescriptorType::CombinedImageSampler, {vk::pipeline::ShaderType::Compute});
        }
        return {device, layout_builder};
    }

    const vk::ShaderModule& ProcessorBuilderVulkan::vk_get_compute_shader() const
    {
        return this->shader->vk_get_compute_shader();
    }

    ProcessorVulkan::ProcessorVulkan(ProcessorBuilderVulkan builder, ProcessorDeviceInfoVulkan device_info):
    descriptor_layout(builder.vk_get_descriptor_set_layout(*device_info.vk_device)),
    compute_pipeline
    (
        vk::pipeline::ShaderStage{builder.vk_get_compute_shader(), vk::pipeline::ShaderType::Compute},
        *device_info.vk_device,
        vk::pipeline::Layout{*device_info.vk_device, vk::DescriptorSetLayoutRefs{this->descriptor_layout}}
    )
    {}

    std::size_t ProcessorVulkan::resource_count() const
    {
        return 0;
    }

    std::size_t ProcessorVulkan::resource_count_of(ResourceType type) const
    {
        (void)type;
        return 0;
    }

    IResource* ProcessorVulkan::get_resource(ResourceHandle handle)
    {
        (void)handle;
        return nullptr;
    }

    void ProcessorVulkan::process()
    {
        std::printf("ayy lmao\n");
    }
}

#endif // TZ_VULKAN