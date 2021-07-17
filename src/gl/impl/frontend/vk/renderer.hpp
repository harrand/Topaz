#ifndef TOPAZ_GL_IMPL_VK_RENDERER_HPP
#define TOPAZ_GL_IMPL_VK_RENDERER_HPP
#if TZ_VULKAN
#include "gl/api/renderer.hpp"
#include "gl/impl/frontend/common/device.hpp"

#include "gl/impl/backend/vk/pipeline/graphics_pipeline.hpp"
#include "gl/impl/backend/vk/logical_device.hpp"
#include "gl/impl/backend/vk/pipeline/shader_module.hpp"
#include "gl/impl/backend/vk/swapchain.hpp"
#include "gl/impl/backend/vk/framebuffer.hpp"
#include "gl/impl/backend/vk/frame_admin.hpp"

namespace tz::gl
{
    class RendererBuilderVulkan : public IRendererBuilder
    {
    public:
        RendererBuilderVulkan() = default;
        //virtual void set_input(const IRendererInput& input) final;
        virtual RendererInputHandle add_input(const IRendererInput& input) final;
        virtual const IRendererInput* get_input(RendererInputHandle handle) const final;

        virtual void set_pass(RenderPassAttachment pass) final;
        virtual RenderPassAttachment get_pass() const final;

        virtual void set_output(const IRendererOutput& output) final;
        virtual const IRendererOutput* get_output() const final;

        virtual ResourceHandle add_resource(const IResource& resource) final;

        virtual void set_culling_strategy(RendererCullingStrategy culling_strategy) final;
        virtual RendererCullingStrategy get_culling_strategy() const final;
        virtual void set_shader(const Shader& shader) final;
        virtual const Shader& get_shader() const final;

        vk::pipeline::VertexInputState vk_get_vertex_input() const;
        vk::pipeline::RasteriserState vk_get_rasteriser_state() const;
        vk::DescriptorSetLayout vk_get_descriptor_set_layout(const vk::LogicalDevice& device) const;
        std::span<const IRendererInput* const> vk_get_inputs() const;
        std::span<const IResource* const> vk_get_buffer_resources() const;
        std::span<const IResource* const> vk_get_texture_resources() const;
    private:
        std::vector<const IRendererInput*> inputs;
        RenderPassAttachment pass = RenderPassAttachment::ColourDepth;
        const IRendererOutput* output = nullptr;
        std::vector<const IResource*> buffer_resources;
        std::vector<const IResource*> texture_resources;
        RendererCullingStrategy culling_strategy = RendererCullingStrategy::NoCulling;
        const Shader* shader = nullptr;
    };

    class DeviceFunctionalityVulkan;

    struct RendererBuilderDeviceInfoVulkan
    {
        const DeviceFunctionalityVulkan* creator_device;
        const vk::LogicalDevice* device;
        vk::pipeline::PrimitiveTopology primitive_type;
        const DeviceWindowBufferVulkan* device_swapchain;
        DeviceWindowResizeCallback* on_resize;
    };

    class RendererPipelineManagerVulkan
    {
    public:
        RendererPipelineManagerVulkan(RendererBuilderVulkan builder, RendererBuilderDeviceInfoVulkan device_info, const RenderPass& render_pass);
        void reconstruct_pipeline();
        const vk::GraphicsPipeline& get_pipeline() const;
        const vk::DescriptorSetLayout& get_resource_descriptor_layout() const;
        const vk::pipeline::Layout& get_layout() const;
    private:
        vk::GraphicsPipeline create_pipeline() const;
        const vk::LogicalDevice* device;
        const RenderPass* render_pass;
        const vk::ShaderModule* vertex_shader;
        const vk::ShaderModule* fragment_shader;
        vk::pipeline::VertexInputState vertex_input_state;
        vk::pipeline::InputAssembly input_assembly;
        vk::pipeline::RasteriserState rasteriser_state;
        const DeviceWindowBufferVulkan* swapchain;
        vk::DescriptorSetLayout resource_descriptor_layout;
        vk::pipeline::Layout layout;
        vk::GraphicsPipeline graphics_pipeline;
    };

    /// Buffer Components represent the guts of an existing Buffer Resource. Only the implementation should be concerned with buffer components -- It is the buffer resource which is user-facing.
    struct BufferComponentVulkan
    {
        vk::Buffer buffer;
        IResource* resource;
    };

    /**
     * @brief Owns and controls all buffers associated with a Renderer. Renderers have multiple buffers used to store input data, and zero or more resource buffers.
     * 
     * Buffer Resources are user-facing descriptions of what a buffer should be like. Buffer components are created by the renderer's buffer manager and represent the implementation-side of the resource. Each buffer resource has exactly one buffer component.
     */
    class RendererBufferManagerVulkan
    {
    public:
        /**
         * @brief Construct a new buffer manager.
         * 
         * @param device_info Information about the Device.
         * @param renderer_inputs List of all inputs. We should sort the input data into buffers.
         */
        RendererBufferManagerVulkan(RendererBuilderDeviceInfoVulkan device_info, std::vector<IRendererInput*> renderer_inputs);
        /**
         * @brief Create empty buffer components for each buffer resource.
         * 
         * @param renderer_buffer_resources List of all buffer resources.
         */
        void initialise_resources(std::vector<IResource*> renderer_buffer_resources);
        /**
         * @brief Fill all of the buffers prepared earlier. This includes the buffers for input data and all buffer components.
         * 
         */
        void setup_buffers();
        const vk::Buffer& get_vertex_buffer() const;
        vk::Buffer& get_vertex_buffer();
        
        const vk::Buffer& get_index_buffer() const;
        vk::Buffer& get_index_buffer();

        const vk::Buffer& get_dynamic_vertex_buffer() const;
        vk::Buffer& get_dynamic_vertex_buffer();

        const vk::Buffer& get_dynamic_index_buffer() const;
        vk::Buffer& get_dynamic_index_buffer();
        /**
         * @brief Retrieve all of the buffer components. If there are no buffer resources, this will be empty.
         * @note `get_buffer_components()[handle]` will retrieve the corresponding BufferComponent for the BufferResource created via the handle `handle`.
         * 
         * @return Span representing all buffer components.
         */
        std::span<const BufferComponentVulkan> get_buffer_components() const;
        /**
         * @brief Retrieve all of the buffer components. If there are no buffer resources, this will be empty.
         * @note `get_buffer_components()[handle]` will retrieve the corresponding BufferComponent for the BufferResource created via the handle `handle`.
         * 
         * @return Span representing all buffer components.
         */
        std::span<BufferComponentVulkan> get_buffer_components();
    private:
        const vk::LogicalDevice* device;
        const vk::hardware::Device* physical_device;
        std::vector<IRendererInput*> inputs;
        vk::Buffer vertex_buffer;
        vk::Buffer dynamic_vertex_buffer;
        vk::Buffer index_buffer;
        vk::Buffer dynamic_index_buffer;
        std::vector<BufferComponentVulkan> buffer_components;
    };

    /// Texture Components represent the guts of an existing Texture Resource. Only the implementation should be concerned with texture components -- It is the texture resource which is user-facing.
    struct TextureComponentVulkan
    {
        vk::Image img;
        vk::ImageView view;
        vk::Sampler sampler;
        IResource* resource;
    };

    class RendererImageManagerVulkan
    {
    public:
        RendererImageManagerVulkan(RendererBuilderVulkan builder, RendererBuilderDeviceInfoVulkan device_info, const RenderPass& render_pass);
        void initialise_resources(std::vector<IResource*> renderer_buffer_resources);
        void setup_depth_image();
        void setup_swapchain_framebuffers();
        std::span<const vk::Framebuffer> get_swapchain_framebuffers() const;
        std::span<const TextureComponentVulkan> get_texture_components() const;
        std::span<TextureComponentVulkan> get_texture_components();
    private:

        const vk::LogicalDevice* device;
        const vk::hardware::Device* physical_device;
        const RenderPass* render_pass;
        const DeviceWindowBufferVulkan* swapchain;
        std::optional<vk::ImageView> maybe_swapchain_offscreen_imageview;
        std::vector<TextureComponentVulkan> texture_components;
        std::optional<vk::Image> depth_image;
        std::optional<vk::ImageView> depth_imageview;
        std::vector<vk::Framebuffer> swapchain_framebuffers;
    };

    class RendererProcessorVulkan
    {
    public:
        RendererProcessorVulkan(RendererBuilderVulkan builder, RendererBuilderDeviceInfoVulkan device_info, std::vector<IRendererInput*> inputs, const RenderPass& render_pass);
        void initialise_resource_descriptors(const RendererPipelineManagerVulkan& pipeline_manager, const RendererBufferManagerVulkan& buffer_manager, const RendererImageManagerVulkan& image_manager, std::vector<const IResource*> resources);
        void initialise_command_pool();
        void block_until_idle();
        void record_rendering_commands(const RendererPipelineManagerVulkan& pipeline_manager, const RendererBufferManagerVulkan& buffer_manager, const RendererImageManagerVulkan& image_manager, tz::Vec4 clear_colour);
        void clear_rendering_commands();
        void record_and_run_scratch_commands(RendererBufferManagerVulkan& buffer_manager, RendererImageManagerVulkan& image_manager);
        void set_regeneration_function(std::function<void()> action);
        void record_draw_list(const RendererDrawList& draws);
        bool draws_match_cache(const RendererDrawList& draws) const;
        void render();
    private:
        std::size_t get_view_count() const;
        std::size_t num_static_inputs() const;
        std::size_t num_dynamic_inputs() const;
        std::size_t num_static_draws() const;
        std::size_t num_dynamic_draws() const;
        RendererDrawList all_inputs_once() const;

        const vk::LogicalDevice* device;
        const vk::hardware::Device* physical_device;
        const RenderPass* render_pass;
        const DeviceWindowBufferVulkan* swapchain;
        std::vector<IRendererInput*> inputs;
        std::optional<vk::DescriptorPool> resource_descriptor_pool;
        vk::CommandPool command_pool;
        vk::hardware::Queue graphics_present_queue;
        std::optional<vk::Buffer> draw_indirect_buffer;
        std::optional<vk::Buffer> draw_indirect_dynamic_buffer;
        RendererDrawList draw_cache;
        vk::FrameAdmin frame_admin;
    };

    class RendererVulkan : public IRenderer
    {
    public:
        constexpr static std::size_t frames_in_flight = 2;
        RendererVulkan(RendererBuilderVulkan builder, RendererBuilderDeviceInfoVulkan device_info);
    
        virtual void set_clear_colour(tz::Vec4 clear_colour) final;
        virtual tz::Vec4 get_clear_colour() const final;
        
        virtual std::size_t input_count() const final;
        virtual std::size_t input_count_of(RendererInputDataAccess access) const final;
        
        virtual IRendererInput* get_input(RendererInputHandle handle) final;
        
        virtual std::size_t resource_count() const final;
        virtual std::size_t resource_count_of(ResourceType type) const final;
        
        virtual IResource* get_resource(ResourceHandle handle) final;
        
        virtual void render() final;
        virtual void render(RendererDrawList draws) final;
    private:
        RenderPass make_simple_render_pass(const RendererBuilderVulkan& builder, const RendererBuilderDeviceInfoVulkan& device_info) const;
        std::vector<std::unique_ptr<IRendererInput>> copy_inputs(const RendererBuilderVulkan builder);
        std::vector<IRendererInput*> get_inputs();
        void handle_resize();
        void handle_clear_colour_change();

        std::vector<std::unique_ptr<IRendererInput>> renderer_inputs;
        std::vector<std::unique_ptr<IResource>> renderer_resources;

        RenderPass render_pass;
        RendererBufferManagerVulkan buffer_manager;
        RendererPipelineManagerVulkan pipeline_manager;
        RendererImageManagerVulkan image_manager;
        RendererProcessorVulkan processor;
        tz::Vec4 clear_colour;
        bool requires_depth_image;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_VK_RENDERER_HPP