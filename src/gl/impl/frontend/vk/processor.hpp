#ifndef TOPAZ_GL_IMPL_FRONTEND_VK_PROCESSOR_HPP
#define TOPAZ_GL_IMPL_FRONTEND_VK_PROCESSOR_HPP
#if TZ_VULKAN
#include "gl/impl/frontend/common/processor.hpp"
#include "gl/impl/frontend/common/component.hpp"
#include "gl/impl/frontend/vk/component.hpp"
#include "gl/impl/backend/vk/pipeline/compute_pipeline.hpp"
#include "gl/impl/backend/vk/descriptor_set_layout.hpp"
#include "gl/impl/backend/vk/fence.hpp"

namespace tz::gl
{
    class ProcessorBuilderVulkan : public ProcessorBuilderBase
    {
    public:
        ProcessorBuilderVulkan() = default;
        vk::DescriptorSetLayout vk_get_descriptor_set_layout(const vk::LogicalDevice& device) const;
        const vk::ShaderModule& vk_get_compute_shader() const;
        std::span<const IResource* const> vk_get_buffer_resources() const;
        std::span<const IResource* const> vk_get_texture_resources() const;
    };

    struct ProcessorDeviceInfoVulkan
    {
        const vk::LogicalDevice* vk_device;
    };

    class ProcessorResourceManagerVulkan
    {
    public:
        ProcessorResourceManagerVulkan(ProcessorBuilderVulkan builder, ProcessorDeviceInfoVulkan device_info, const vk::DescriptorSetLayout& descriptor_layout);
        std::size_t resource_count() const;
        std::size_t resource_count_of(ResourceType type) const;
        IResource* get_resource(ResourceHandle handle);
        const vk::DescriptorPool* get_descriptor_pool() const;

        BufferComponentVulkan* get_buffer_component(std::size_t buffer_id);
        TextureComponentVulkan* get_texture_component(std::size_t texture_id);
    private:
        void setup_buffers();
        void setup_textures();
        void initialise_resource_descriptors();

        const vk::LogicalDevice* device;
        const vk::DescriptorSetLayout* descriptor_layout;
        const ShaderVulkan* shader;
        std::vector<std::unique_ptr<IResource>> buffer_resources;
        std::vector<std::unique_ptr<IResource>> texture_resources;
        /*
        std::vector<BufferComponentVulkan> buffer_components;
        std::vector<TextureComponentVulkan> texture_components;
        */
        ComponentManager components;
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
        void record_and_run_scratch_commands();

        const vk::LogicalDevice* device;

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