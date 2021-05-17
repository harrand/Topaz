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

        virtual void set_culling_strategy(RendererCullingStrategy culling_strategy) final;
        virtual RendererCullingStrategy get_culling_strategy() const final;
        virtual void set_render_pass(const RenderPass& render_pass) final;
        virtual const RenderPass& get_render_pass() const final;
        virtual void set_shader(const Shader& shader) final;
        virtual const Shader& get_shader() const final;

        vk::pipeline::VertexInputState vk_get_vertex_input() const;
        vk::pipeline::RasteriserState vk_get_rasteriser_state() const;
    private:
        const IRendererInput* input = nullptr;
        const IRendererOutput* output = nullptr;
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

    class RendererVulkan : public IRenderer
    {
    public:
        constexpr static std::size_t frames_in_flight = 2;
        RendererVulkan(RendererBuilderVulkan builder, RendererBuilderDeviceInfoVulkan);
    
        virtual void set_clear_colour(tz::Vec4 clear_colour) final;
        virtual tz::Vec4 get_clear_colour() const final;
        
        virtual void render() final;
    private:
        void setup_buffers(const IRendererInput* input);
        void setup_depth_image();
        void setup_swapchain_framebuffers(const RenderPass& render_pass);
        void record_rendering_commands(const RenderPass& render_pass, const IRendererInput* input);
        void record_and_run_scratch_commands(const IRendererInput* input);

        void handle_resize();

        const vk::LogicalDevice* device;
        const vk::hardware::Device* physical_device;
        vk::hardware::MemoryModule device_local_mem;
        vk::hardware::MemoryModule host_visible_mem;
        const vk::ShaderModule* vertex_shader;
        const vk::ShaderModule* fragment_shader;
        vk::pipeline::VertexInputState vertex_input_state;
        vk::pipeline::InputAssembly input_assembly;
        vk::pipeline::RasteriserState rasteriser_state;
        const RenderPass* render_pass;
        const IRendererInput* renderer_input;
        vk::GraphicsPipeline graphics_pipeline;
        std::optional<vk::Buffer> vertex_buffer;
        std::optional<vk::Buffer> index_buffer;
        std::optional<vk::Image> depth_image;
        std::optional<vk::ImageView> depth_imageview;
        vk::CommandPool command_pool;
        const vk::Swapchain* swapchain;
        std::vector<vk::Framebuffer> swapchain_framebuffers;
        vk::hardware::Queue graphics_present_queue;
        tz::Vec4 clear_colour;
        vk::FrameAdmin frame_admin;
    };
}

#endif // TOPAZ_GL_IMPL_VK_RENDERER_HPP