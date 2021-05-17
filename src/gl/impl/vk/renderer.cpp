#if TZ_VULKAN
#include "core/report.hpp"
#include "gl/impl/vk/renderer.hpp"
#include "gl/impl/vk/device.hpp"
#include "gl/vk/fence.hpp"
#include "gl/vk/submit.hpp"

namespace tz::gl
{
    void RendererBuilderVulkan::set_input(const IRendererInput& input)
    {
        this->input = &input;
    }

    const IRendererInput* RendererBuilderVulkan::get_input() const
    {
        return this->input;
    }

    void RendererBuilderVulkan::set_output(const IRendererOutput& output)
    {
        this->output = &output;
    }

    const IRendererOutput* RendererBuilderVulkan::get_output() const
    {
        return this->output;
    }

    void RendererBuilderVulkan::set_culling_strategy(RendererCullingStrategy culling_strategy)
    {
        this->culling_strategy = culling_strategy;
    }

    RendererCullingStrategy RendererBuilderVulkan::get_culling_strategy() const
    {
        return this->culling_strategy;
    }

    void RendererBuilderVulkan::set_render_pass(const RenderPass& render_pass)
    {
        this->render_pass = &render_pass;
    }

    const RenderPass& RendererBuilder::get_render_pass() const
    {
        tz_assert(this->render_pass != nullptr, "No render pass set");
        return *this->render_pass;
    }

    void RendererBuilderVulkan::set_shader(const Shader& shader)
    {
        this->shader = &shader;
    }

    const Shader& RendererBuilderVulkan::get_shader() const
    {
        tz_assert(this->shader != nullptr, "No shader yet");
        return *this->shader;
    }

    vk::pipeline::VertexInputState RendererBuilderVulkan::vk_get_vertex_input() const
    {
        if(this->input == nullptr)
        {
            // Just default vertexinput as we don't have any inputs.
            return {};
        }

        // Do the work to retrieve the formats and build up the vertex input state.
        RendererElementFormat fmt = this->input->get_format();
        vk::VertexInputRate input_rate;
        switch(fmt.basis)
        {
            case RendererInputFrequency::PerVertexBasis:
                input_rate = vk::VertexInputRate::PerVertexBasis;
            break;
            case RendererInputFrequency::PerInstanceBasis:
                input_rate = vk::VertexInputRate::PerInstanceBasis;
            break;
            default:
                tz_error("Can't map RendererInputFrequency to vk::VertexInputRate");
            break;
        }
        vk::VertexBindingDescription binding{0, fmt.binding_size, input_rate};
        vk::VertexAttributeDescriptions attributes;
        for(std::size_t i = 0; i < fmt.binding_attributes.length(); i++)
        {
            RendererAttributeFormat attribute = fmt.binding_attributes[i];
            VkFormat vk_format;
            switch(attribute.type)
            {
                case RendererComponentType::Float32:
                    vk_format = VK_FORMAT_R32_SFLOAT;
                break;
                case RendererComponentType::Float32x2:
                    vk_format = VK_FORMAT_R32G32_SFLOAT;
                break;
                case RendererComponentType::Float32x3:
                    vk_format = VK_FORMAT_R32G32B32_SFLOAT;
                break;
                default:
                    tz_error("Unknown mapping from RendererComponentType to VkFormat");
                    vk_format = VK_FORMAT_UNDEFINED;
                break;
            }
            attributes.emplace_back(0, i, vk_format, attribute.element_attribute_offset);
        }
        return {vk::VertexBindingDescriptions{binding}, attributes};
    }

    vk::pipeline::RasteriserState RendererBuilderVulkan::vk_get_rasteriser_state() const
    {
        vk::pipeline::CullingStrategy vk_cull;
        switch(this->culling_strategy)
        {
            case RendererCullingStrategy::NoCulling:
                vk_cull = vk::pipeline::CullingStrategy::None;
            break;
            case RendererCullingStrategy::CullFrontFaces:
                vk_cull = vk::pipeline::CullingStrategy::Front;
            break;
            case RendererCullingStrategy::CullBackFaces:
                vk_cull = vk::pipeline::CullingStrategy::Back;
            break;
            case RendererCullingStrategy::CullEverything:
                vk_cull = vk::pipeline::CullingStrategy::Both;
            break;
        }
        return
        {
            false,
            false,
            vk::pipeline::PolygonMode::Fill,
            1.0f,
            vk_cull
        };
    }

    RendererVulkan::RendererVulkan(RendererBuilderVulkan builder, RendererBuilderDeviceInfoVulkan device_info):
    graphics_pipeline
    (
        {vk::pipeline::ShaderStage{builder.get_shader().vk_get_vertex_shader(), vk::pipeline::ShaderType::Vertex}, vk::pipeline::ShaderStage{builder.get_shader().vk_get_fragment_shader(), vk::pipeline::ShaderType::Fragment}},
        *device_info.device,
        builder.vk_get_vertex_input(),
        vk::pipeline::InputAssembly{device_info.primitive_type},
        vk::pipeline::ViewportState{*device_info.device_swapchain, true},
        builder.vk_get_rasteriser_state(),
        vk::pipeline::MultisampleState{},
        vk::pipeline::ColourBlendState{},
        vk::pipeline::DynamicState::None(),
        vk::pipeline::Layout{*device_info.device},
        builder.get_render_pass().vk_get_render_pass()
    ),
    vertex_buffer(std::nullopt),
    index_buffer(std::nullopt),
    depth_image(std::nullopt),
    depth_imageview(std::nullopt),
    command_pool(*device_info.device, device_info.device->get_queue_family(), vk::CommandPool::RecycleBuffer),
    swapchain(device_info.device_swapchain),
    swapchain_framebuffers(),
    graphics_present_queue(device_info.device->get_hardware_queue()),
    clear_colour(),
    frame_admin(*device_info.device, RendererVulkan::frames_in_flight)
    {
        this->clear_colour = {0.0f, 0.0f, 0.0f, 0.0f};
        const vk::LogicalDevice& device = *device_info.device;
        const vk::hardware::Device& physical_device = *device.get_queue_family().dev;
        vk::hardware::MemoryModule host_visible_mem = physical_device.get_memory_properties().unsafe_get_some_module_matching({vk::hardware::MemoryType::HostVisible, vk::hardware::MemoryType::HostCoherent});
        vk::hardware::MemoryModule device_local_mem = physical_device.get_memory_properties().unsafe_get_some_module_matching({vk::hardware::MemoryType::DeviceLocal});

        this->vertex_buffer = vk::Buffer{vk::BufferType::Vertex, vk::BufferPurpose::TransferDestination, device, device_local_mem, builder.get_input()->get_vertex_bytes().size_bytes()};
        this->index_buffer = vk::Buffer{vk::BufferType::Index, vk::BufferPurpose::TransferDestination, device, device_local_mem, builder.get_input()->get_indices().size_bytes()};

        auto swapchain_width = static_cast<std::uint32_t>(this->swapchain->get_width());
        auto swapchain_height = static_cast<std::uint32_t>(this->swapchain->get_height());
        this->depth_image = vk::Image{device, swapchain_width, swapchain_height, vk::Image::Format::DepthFloat32, {vk::Image::Usage::DepthStencilAttachment}, device_local_mem};
        this->depth_imageview = vk::ImageView{device, this->depth_image.value()};

        for(const vk::ImageView& swapchain_view : this->swapchain->get_image_views())
        {
            this->swapchain_framebuffers.emplace_back(builder.get_render_pass().vk_get_render_pass(), swapchain_view, this->depth_imageview.value(), VkExtent2D{swapchain_width, swapchain_height});
        }

        // Command Buffers for each swapchain image, but an extra general-purpose recycleable buffer.
        constexpr std::size_t num_scratch_command_bufs = 1;
        this->command_pool.with(this->swapchain->get_image_views().size() + num_scratch_command_bufs);
        // Now setup the swapchain image buffers
        VkClearValue vk_clear_colour{this->clear_colour[0], this->clear_colour[1], this->clear_colour[2], this->clear_colour[3]};
        for(std::size_t i = 0; i < this->swapchain->get_image_views().size(); i++)
        {
            vk::CommandBufferRecording render = this->command_pool[i].record();
            vk::RenderPassRun run{this->command_pool[i], builder.get_render_pass().vk_get_render_pass(), this->swapchain_framebuffers[i], this->swapchain->full_render_area(), vk_clear_colour};
            this->graphics_pipeline.bind(this->command_pool[i]);
            render.bind(this->vertex_buffer.value());
            render.bind(this->index_buffer.value());
            auto indices_count = builder.get_input()->get_indices().size();
            render.draw_indexed(indices_count);
        }

        // Setup transfers using the scratch buffers.
        vk::CommandBuffer& scratch_buf = command_pool[this->swapchain->get_image_views().size()];
        {
            // Part 1: Transfer vertex data.
            auto vertex_data = builder.get_input()->get_vertex_bytes();
            tz_report("VB (%zu vertices, %zu bytes total)", vertex_data.size(), vertex_data.size_bytes());
            vk::Buffer vertices_staging{vk::BufferType::Staging, vk::BufferPurpose::TransferSource, device, host_visible_mem, vertex_data.size_bytes()};
            vertices_staging.write(vertex_data.data(), vertex_data.size_bytes());
            {
                vk::CommandBufferRecording transfer_vertices = scratch_buf.record();
                transfer_vertices.buffer_copy_buffer(vertices_staging, this->vertex_buffer.value(), vertex_data.size_bytes());
            }

            vk::Fence copy_fence{device};
            copy_fence.signal();
            vk::Submit do_scratch_operation{vk::CommandBuffers{scratch_buf}, vk::SemaphoreRefs{}, vk::WaitStages{}, vk::SemaphoreRefs{}};

            // Submit Part 1
            do_scratch_operation(this->graphics_present_queue, copy_fence);

            copy_fence.wait_for();
            scratch_buf.reset();
            // Part 2: Transfer index data.
            auto index_data = builder.get_input()->get_indices();
            tz_report("IB (%zu indices, %zu bytes total)", index_data.size(), index_data.size_bytes());
            vk::Buffer indices_staging{vk::BufferType::Staging, vk::BufferPurpose::TransferSource, device, host_visible_mem, index_data.size_bytes()};
            indices_staging.write(index_data.data(), index_data.size_bytes());
            {
                vk::CommandBufferRecording transfer_indices = scratch_buf.record();
                transfer_indices.buffer_copy_buffer(indices_staging, this->index_buffer.value(), index_data.size_bytes());
            }
            copy_fence.signal();
            // Submit Part 2
            do_scratch_operation(this->graphics_present_queue, copy_fence);
            copy_fence.wait_for();
        }

        tz_report("RendererVulkan (Input = %p)", builder.get_input());
    }

    void RendererVulkan::set_clear_colour(tz::Vec4 clear_colour)
    {
        this->clear_colour = clear_colour;
        tz_error("Setting clear-colour post-creation is not yet implemented vulkan-side. Sorry");
    }

    tz::Vec4 RendererVulkan::get_clear_colour() const
    {
        return this->clear_colour;
    }

    void RendererVulkan::render()
    {
        this->frame_admin.render_frame(this->graphics_present_queue, *this->swapchain, this->command_pool, vk::WaitStages{vk::WaitStage::ColourAttachmentOutput});
    }
}
#endif // TZ_VULKAN