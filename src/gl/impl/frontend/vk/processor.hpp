#ifndef TOPAZ_GL_IMPL_FRONTEND_VK_PROCESSOR_HPP
#define TOPAZ_GL_IMPL_FRONTEND_VK_PROCESSOR_HPP
#if TZ_VULKAN
#include "gl/api/processor.hpp"
#include "gl/impl/frontend/vk/component.hpp"
#include "gl/impl/backend/vk/pipeline/compute_pipeline.hpp"
#include "gl/impl/backend/vk/descriptor_set_layout.hpp"
#include "gl/impl/backend/vk/fence.hpp"

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
        std::span<const IResource* const> vk_get_buffer_resources() const;
        std::span<const IResource* const> vk_get_texture_resources() const;
    private:
        std::vector<const IResource*> buffer_resources;
        std::vector<const IResource*> texture_resources;
        const Shader* shader = nullptr;
    };

    struct ProcessorDeviceInfoVulkan
    {
        const vk::LogicalDevice* vk_device;
    };

    class ProcessorResourceManagerVulkan
    {
    public:
        ProcessorResourceManagerVulkan(ProcessorBuilderVulkan builder, ProcessorDeviceInfoVulkan device_info, const vk::DescriptorSetLayout& descriptor_layout, const vk::pipeline::Layout& pipeline_layout);
        std::size_t resource_count() const;
        std::size_t resource_count_of(ResourceType type) const;
        IResource* get_resource(ResourceHandle handle);
        const vk::DescriptorPool* get_descriptor_pool() const;
    private:
        void setup_buffers();
        void setup_textures();
        void initialise_resource_descriptors();

        const vk::LogicalDevice* device;
        const vk::DescriptorSetLayout* descriptor_layout;
        const vk::pipeline::Layout* pipeline_layout;
        std::vector<std::unique_ptr<IResource>> buffer_resources;
        std::vector<std::unique_ptr<IResource>> texture_resources;
        std::vector<BufferComponentVulkan> buffer_components;
        std::vector<TextureComponentVulkan> texture_components;
        std::optional<vk::DescriptorPool> resource_descriptor_pool;
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
        void record_processing_commands();

        vk::DescriptorSetLayout descriptor_layout;
        vk::pipeline::Layout pipeline_layout;
        vk::ComputePipeline compute_pipeline;
        ProcessorResourceManagerVulkan resource_manager;

        vk::CommandPool command_pool;
        vk::hardware::Queue compute_queue;
        vk::Fence process_fence;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_FRONTEND_VK_PROCESSOR_HPP