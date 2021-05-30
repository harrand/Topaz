#ifndef TOPAZ_GL_IMPL_VK_RENDERER_HPP
#define TOPAZ_GL_IMPL_VK_RENDERER_HPP
#include "gl/api/renderer.hpp"
#include "gl/impl/common/device.hpp"

#include "gl/vk/pipeline/graphics_pipeline.hpp"

#include "gl/vk/logical_device.hpp"
#include "gl/vk/pipeline/shader_module.hpp"
#include "gl/vk/swapchain.hpp"
#include "gl/vk/framebuffer.hpp"
#include "gl/vk/frame_admin.hpp"

namespace tz::gl
{
    class RendererBuilderVulkan : public IRendererBuilder
    {
    public:
        RendererBuilderVulkan() = default;
        virtual void set_input(const IRendererInput& input) final;
        virtual const IRendererInput* get_input() const final;

        virtual void set_output(const IRendererOutput& output) final;
        virtual const IRendererOutput* get_output() const final;

        virtual ResourceHandle add_resource(const IResource& resource) final;

        virtual void set_culling_strategy(RendererCullingStrategy culling_strategy) final;
        virtual RendererCullingStrategy get_culling_strategy() const final;
        virtual void set_render_pass(const RenderPass& render_pass) final;
        virtual const RenderPass& get_render_pass() const final;
        virtual void set_shader(const Shader& shader) final;
        virtual const Shader& get_shader() const final;

        vk::pipeline::VertexInputState vk_get_vertex_input() const;
        vk::pipeline::RasteriserState vk_get_rasteriser_state() const;
        vk::DescriptorSetLayout vk_get_descriptor_set_layout(const vk::LogicalDevice& device) const;
        std::span<const IResource* const> vk_get_buffer_resources() const;
        std::span<const IResource* const> vk_get_texture_resources() const;
    private:
        const IRendererInput* input = nullptr;
        const IRendererOutput* output = nullptr;
        std::vector<const IResource*> buffer_resources;
        std::vector<const IResource*> texture_resources;
        RendererCullingStrategy culling_strategy = RendererCullingStrategy::NoCulling;
        const RenderPass* render_pass = nullptr;
        const Shader* shader = nullptr;
    };

    struct RendererBuilderDeviceInfoVulkan
    {
        const vk::LogicalDevice* device;
        vk::pipeline::PrimitiveTopology primitive_type;
        const vk::Swapchain* device_swapchain;
        DeviceWindowResizeCallback* on_resize;
    };

    class RendererPipelineManagerVulkan
    {
    public:
        RendererPipelineManagerVulkan(RendererBuilderVulkan builder, RendererBuilderDeviceInfoVulkan device_info);
        void reconstruct_pipeline();
        const vk::GraphicsPipeline& get_pipeline() const;
        const vk::DescriptorSetLayout* get_resource_descriptor_layout() const;
        const vk::pipeline::Layout& get_layout() const;
    private:
        const vk::LogicalDevice* device;
        const RenderPass* render_pass;
        const vk::ShaderModule* vertex_shader;
        const vk::ShaderModule* fragment_shader;
        vk::pipeline::VertexInputState vertex_input_state;
        vk::pipeline::InputAssembly input_assembly;
        vk::pipeline::RasteriserState rasteriser_state;
        const vk::Swapchain* swapchain;
        vk::DescriptorSetLayout resource_descriptor_layout;
        vk::pipeline::Layout layout;
        vk::GraphicsPipeline graphics_pipeline;
    };

    class RendererBufferManagerVulkan
    {
    public:
        RendererBufferManagerVulkan(RendererBuilderDeviceInfoVulkan device_info, IRendererInput* renderer_input);
        void initialise_resources(std::vector<IResource*> renderer_buffer_resources);
        void setup_buffers();
        const vk::Buffer* get_vertex_buffer() const;
        vk::Buffer* get_vertex_buffer();
        const vk::Buffer* get_index_buffer() const;
        vk::Buffer* get_index_buffer();
        std::span<const IResource* const> get_buffer_resources() const;
        std::span<IResource*> get_buffer_resources();
        std::span<const vk::Buffer> get_resource_buffers() const;
        std::span<vk::Buffer> get_resource_buffers();
    private:
        const vk::LogicalDevice* device;
        const vk::hardware::Device* physical_device;
        IRendererInput* input;        
        std::optional<vk::Buffer> vertex_buffer;
        std::optional<vk::Buffer> index_buffer;
        std::vector<IResource*> buffer_resources;
        std::vector<vk::Buffer> buffer_resource_buffers;
    };

    class RendererImageManagerVulkan
    {
    public:
        RendererImageManagerVulkan(RendererBuilderVulkan builder, RendererBuilderDeviceInfoVulkan device_info);
        void setup_depth_image();
        void setup_swapchain_framebuffers();
        std::span<const vk::Framebuffer> get_swapchain_framebuffers() const;
    private:
        const vk::LogicalDevice* device;
        const vk::hardware::Device* physical_device;
        const RenderPass* render_pass;
        const vk::Swapchain* swapchain;
        std::optional<vk::Image> depth_image;
        std::optional<vk::ImageView> depth_imageview;
        std::vector<vk::Framebuffer> swapchain_framebuffers;
    };

    class RendererProcessorVulkan
    {
    public:
        RendererProcessorVulkan(RendererBuilderVulkan builder, RendererBuilderDeviceInfoVulkan device_info, const IRendererInput* input);
        void initialise_resource_descriptors(const RendererPipelineManagerVulkan& pipeline_manager, const RendererBufferManagerVulkan& buffer_manager, std::vector<const IResource*> resources);
        void initialise_command_pool();
        void block_until_idle();
        void record_rendering_commands(const RendererPipelineManagerVulkan& pipeline_manager, const RendererBufferManagerVulkan& buffer_manager, const RendererImageManagerVulkan& image_manager, tz::Vec4 clear_colour);
        void record_and_run_scratch_commands(RendererBufferManagerVulkan& buffer_manager);
        void set_regeneration_function(std::function<void()> action);
        void render();
    private:
        const vk::LogicalDevice* device;
        const vk::hardware::Device* physical_device;
        const RenderPass* render_pass;
        const vk::Swapchain* swapchain;
        const IRendererInput* input;
        std::optional<vk::DescriptorPool> resource_descriptor_pool;
        vk::CommandPool command_pool;
        vk::hardware::Queue graphics_present_queue;
        vk::FrameAdmin frame_admin;
    };

    class RendererVulkan : public IRenderer
    {
    public:
        constexpr static std::size_t frames_in_flight = 2;
        RendererVulkan(RendererBuilderVulkan builder, RendererBuilderDeviceInfoVulkan device_info);
    
        virtual void set_clear_colour(tz::Vec4 clear_colour) final;
        virtual tz::Vec4 get_clear_colour() const final;
        virtual IRendererInput* get_input() final;
        virtual IResource* get_resource(ResourceHandle handle) final;
        
        virtual void render() final;
    private:
        void handle_resize();
        void handle_clear_colour_change();

        std::unique_ptr<IRendererInput> renderer_input;
        std::vector<std::unique_ptr<IResource>> renderer_resources;

        RendererBufferManagerVulkan buffer_manager;
        RendererPipelineManagerVulkan pipeline_manager;
        RendererImageManagerVulkan image_manager;
        RendererProcessorVulkan processor;
        tz::Vec4 clear_colour;
    };
}

#endif // TOPAZ_GL_IMPL_VK_RENDERER_HPP