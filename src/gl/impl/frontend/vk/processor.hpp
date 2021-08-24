#ifndef TOPAZ_GL_IMPL_FRONTEND_VK_PROCESSOR_HPP
#define TOPAZ_GL_IMPL_FRONTEND_VK_PROCESSOR_HPP
#if TZ_VULKAN
#include "gl/api/processor.hpp"
#include "gl/impl/backend/vk/pipeline/compute_pipeline.hpp"
#include "gl/impl/backend/vk/descriptor_set_layout.hpp"

namespace tz::gl
{
    class ProcessorBuilderVulkan : public IProcessorBuilder
    {
    public:
        ProcessorBuilderVulkan() = default;
        virtual ResourceHandle add_resource(const IResource& resource) final;
        virtual const IResource* get_resource(ResourceHandle handle) const final;
        virtual void set_shader(const Shader& shader) final;
        virtual const Shader& get_shader() const final;

        vk::DescriptorSetLayout vk_get_descriptor_set_layout(const vk::LogicalDevice& device) const;
        const vk::ShaderModule& vk_get_compute_shader() const;
    private:
        std::vector<const IResource*> buffer_resources;
        std::vector<const IResource*> texture_resources;
        const Shader* shader = nullptr;
    };

    struct ProcessorDeviceInfoVulkan
    {
        const vk::LogicalDevice* vk_device;
    };

    class ProcessorVulkan : public IProcessor
    {
    public:
        ProcessorVulkan(ProcessorBuilderVulkan builder, ProcessorDeviceInfoVulkan device_info);
        virtual std::size_t resource_count() const final;
        virtual std::size_t resource_count_of(ResourceType type) const final;
        virtual IResource* get_resource(ResourceHandle handle) final;
        virtual void process() final;
    private:
        vk::DescriptorSetLayout descriptor_layout;
        vk::ComputePipeline compute_pipeline;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_FRONTEND_VK_PROCESSOR_HPP