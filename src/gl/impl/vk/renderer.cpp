#if TZ_VULKAN
#include "core/report.hpp"
#include "gl/impl/vk/renderer.hpp"
#include "gl/impl/vk/device.hpp"
#include "gl/vk/tz_vulkan.hpp"
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

    ResourceHandle RendererBuilderVulkan::add_resource(const IResource& resource)
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

    vk::DescriptorSetLayout RendererBuilderVulkan::vk_get_descriptor_set_layout(const vk::LogicalDevice& device) const
    {
        vk::LayoutBuilder layout_builder;
        for(std::size_t i = 0; i < this->buffer_resources.size(); i++)
        {
            layout_builder.add(vk::DescriptorType::UniformBuffer, vk::pipeline::ShaderTypeField::All());
        }
        for(std::size_t i = this->buffer_resources.size(); i < this->buffer_resources.size() + this->texture_resources.size(); i++)
        {
            layout_builder.add(vk::DescriptorType::CombinedImageSampler, vk::pipeline::ShaderTypeField::All());
        }
        return {device, layout_builder};
    }

    std::span<const IResource* const> RendererBuilderVulkan::vk_get_buffer_resources() const
    {
        return {this->buffer_resources.begin(), this->buffer_resources.end()};
    }

    std::span<const IResource* const> RendererBuilderVulkan::vk_get_texture_resources() const
    {
        return {this->texture_resources.begin(), this->texture_resources.end()};
    }

    RendererPipelineManagerVulkan::RendererPipelineManagerVulkan(RendererBuilderVulkan builder, RendererBuilderDeviceInfoVulkan device_info):
    device(device_info.device),
    render_pass(&builder.get_render_pass()),
    vertex_shader(&builder.get_shader().vk_get_vertex_shader()),
    fragment_shader(&builder.get_shader().vk_get_fragment_shader()),
    vertex_input_state(builder.vk_get_vertex_input()),
    input_assembly(device_info.primitive_type),
    rasteriser_state(builder.vk_get_rasteriser_state()),
    swapchain(device_info.device_swapchain),
    resource_descriptor_layout(builder.vk_get_descriptor_set_layout(*this->device)),
    layout(*this->device, vk::DescriptorSetLayoutRefs{this->resource_descriptor_layout}),
    graphics_pipeline(this->create_pipeline())
    {
    }

    void RendererPipelineManagerVulkan::reconstruct_pipeline()
    {
        // Assume swapchain has been reinitialised by the device.
        // Assume render_pass has been reinitialised by the device.
        // Recreate everything else.
        this->graphics_pipeline = this->create_pipeline();
    }

    const vk::GraphicsPipeline& RendererPipelineManagerVulkan::get_pipeline() const
    {
        return this->graphics_pipeline;
    }

    const vk::DescriptorSetLayout& RendererPipelineManagerVulkan::get_resource_descriptor_layout() const
    {
        return this->resource_descriptor_layout;
    }

    const vk::pipeline::Layout& RendererPipelineManagerVulkan::get_layout() const
    {
        return this->layout;
    }

    vk::GraphicsPipeline RendererPipelineManagerVulkan::create_pipeline() const
    {
        auto make_viewport_state = [this]()
        {
            if(vk::is_headless())
            {
                const auto& as_image = static_cast<const vk::Image&>(*this->swapchain);
                return vk::pipeline::ViewportState{as_image, true};
            }
            else
            {
                const auto& as_swapchain = static_cast<const vk::Swapchain&>(*this->swapchain);
                return vk::pipeline::ViewportState{as_swapchain, true};
            }
        };
        return vk::GraphicsPipeline
        {
            {vk::pipeline::ShaderStage{*this->vertex_shader, vk::pipeline::ShaderType::Vertex}, vk::pipeline::ShaderStage{*this->fragment_shader, vk::pipeline::ShaderType::Fragment}},
            *this->device,
            this->vertex_input_state,
            this->input_assembly,
            make_viewport_state(),
            this->rasteriser_state,
            vk::pipeline::MultisampleState{},
            vk::pipeline::ColourBlendState{},
            vk::pipeline::DynamicState::None(),
            this->layout,
            this->render_pass->vk_get_render_pass()
        };
    }

    RendererBufferManagerVulkan::RendererBufferManagerVulkan(RendererBuilderDeviceInfoVulkan device_info, IRendererInput* renderer_input):
    device(device_info.device),
    physical_device(this->device->get_queue_family().dev),
    input(renderer_input),
    vertex_buffer(std::nullopt),
    index_buffer(std::nullopt),
    buffer_resources(),
    buffer_resource_buffers()
    {
    }

    void RendererBufferManagerVulkan::initialise_resources(std::vector<IResource*> renderer_buffer_resources)
    {
        this->buffer_resources = renderer_buffer_resources;
    }

    void RendererBufferManagerVulkan::setup_buffers()
    {
        if(this->input != nullptr)
        {
            switch(this->input->data_access())
            {
                case RendererInputDataAccess::StaticFixed:
                    this->vertex_buffer = vk::Buffer{vk::BufferType::Vertex, vk::BufferPurpose::TransferDestination, *this->device, vk::hardware::MemoryResidency::GPU, this->input->get_vertex_bytes().size_bytes()};
                    this->index_buffer = vk::Buffer{vk::BufferType::Index, vk::BufferPurpose::TransferDestination, *this->device, vk::hardware::MemoryResidency::GPU, this->input->get_indices().size_bytes()};
                break;
                case RendererInputDataAccess::DynamicFixed:
                    {
                        auto& dynamic_input = static_cast<IRendererDynamicInput&>(*this->input);
                        // Create buffers in host-visible memory (slow) and pass the mapped ptrs to the renderer input.
                        // Note: This also copies over the initial vertex data to buffers. Nothing is done in scratch command buffers this time.
                        this->vertex_buffer = vk::Buffer{vk::BufferType::Vertex, vk::BufferPurpose::NothingSpecial, *this->device, vk::hardware::MemoryResidency::CPUPersistent, this->input->get_vertex_bytes().size_bytes()};
                        dynamic_input.set_vertex_data(static_cast<std::byte*>(this->vertex_buffer->map_memory()));
                        this->index_buffer = vk::Buffer{vk::BufferType::Index, vk::BufferPurpose::NothingSpecial, *this->device, vk::hardware::MemoryResidency::CPUPersistent, this->input->get_indices().size_bytes()};
                        dynamic_input.set_index_data(static_cast<unsigned int*>(this->index_buffer->map_memory()));
                    }
                break;
                default:
                    tz_error("Input data access unsupported (Vulkan)");
                break;
            }
        }

        this->buffer_resource_buffers.clear();
        for(IResource* buffer_resource : this->buffer_resources)
        {
            switch(buffer_resource->data_access())
            {
                case RendererInputDataAccess::StaticFixed:
                    this->buffer_resource_buffers.emplace_back(vk::BufferType::Uniform, vk::BufferPurpose::TransferDestination, *this->device, vk::hardware::MemoryResidency::GPU, buffer_resource->get_resource_bytes().size_bytes());
                break;
                case RendererInputDataAccess::DynamicFixed:
                    {
                        auto& dynamic_resource = static_cast<IDynamicResource&>(*buffer_resource);
                        vk::Buffer& resource_buf = this->buffer_resource_buffers.emplace_back(vk::BufferType::Uniform, vk::BufferPurpose::NothingSpecial, *this->device, vk::hardware::MemoryResidency::CPUPersistent, dynamic_resource.get_resource_bytes().size_bytes());
                        dynamic_resource.set_resource_data(static_cast<std::byte*>(resource_buf.map_memory()));
                    }
                break;
                default:
                    tz_error("Resource data access unsupported (Vulkan)");
                break;
            }
        }
    }

    const vk::Buffer* RendererBufferManagerVulkan::get_vertex_buffer() const
    {
        if(this->vertex_buffer.has_value())
        {
            return &this->vertex_buffer.value();
        }
        return nullptr;
    }

    vk::Buffer* RendererBufferManagerVulkan::get_vertex_buffer()
    {
        if(this->vertex_buffer.has_value())
        {
            return &this->vertex_buffer.value();
        }
        return nullptr;
    }

    const vk::Buffer* RendererBufferManagerVulkan::get_index_buffer() const
    {
        if(this->index_buffer.has_value())
        {
            return &this->index_buffer.value();
        }
        return nullptr;
    }

    vk::Buffer* RendererBufferManagerVulkan::get_index_buffer()
    {
        if(this->index_buffer.has_value())
        {
            return &this->index_buffer.value();
        }
        return nullptr;
    }

    std::span<const IResource* const> RendererBufferManagerVulkan::get_buffer_resources() const
    {
        return {this->buffer_resources.begin(), this->buffer_resources.end()};
    }

    std::span<IResource*> RendererBufferManagerVulkan::get_buffer_resources()
    {
        return {this->buffer_resources.begin(), this->buffer_resources.end()};
    }

    std::span<const vk::Buffer> RendererBufferManagerVulkan::get_resource_buffers() const
    {
        return {this->buffer_resource_buffers.begin(), this->buffer_resource_buffers.end()};
    }

    std::span<vk::Buffer> RendererBufferManagerVulkan::get_resource_buffers()
    {
        return {this->buffer_resource_buffers.begin(), this->buffer_resource_buffers.end()};
    }

    RendererImageManagerVulkan::RendererImageManagerVulkan(RendererBuilderVulkan builder, RendererBuilderDeviceInfoVulkan device_info):
    device(device_info.device),
    physical_device(this->device->get_queue_family().dev),
    render_pass(&builder.get_render_pass()),
    swapchain(device_info.device_swapchain),
    maybe_swapchain_offscreen_imageview(std::nullopt),
    depth_image(std::nullopt),
    depth_imageview(std::nullopt),
    swapchain_framebuffers()
    {
        if(vk::is_headless())
        {
            this->swapchain_framebuffers.reserve(1);
            this->maybe_swapchain_offscreen_imageview = vk::ImageView{*this->device, static_cast<const vk::Image&>(*this->swapchain)};
        }
        else
        {
            const auto& as_swapchain = static_cast<const vk::Swapchain&>(*this->swapchain);
            this->swapchain_framebuffers.reserve(as_swapchain.get_image_views().size());
        }
        
    }

    void RendererImageManagerVulkan::initialise_resources(std::vector<IResource*> renderer_texture_resources)
    {
        this->texture_resources = renderer_texture_resources;
        for(IResource* texture_resource : this->texture_resources)
        {
            auto* tex_res = static_cast<TextureResource*>(texture_resource);
            vk::Image::Format format;
            switch(tex_res->get_format())
            {
                case TextureFormat::Rgba32Signed:
                    format = vk::Image::Format::Rgba32Signed;
                break;
                case TextureFormat::Rgba32Unsigned:
                    format = vk::Image::Format::Rgba32Unsigned;
                break;
                case TextureFormat::Rgba32sRGB:
                    format = vk::Image::Format::Rgba32sRGB;
                break;
                case TextureFormat::DepthFloat32:
                    format = vk::Image::Format::DepthFloat32;
                break;
                default:
                    tz_error("Unrecognised texture format (Vulkan)");
                break;
            }

            vk::Image img{*this->device, tex_res->get_width(), tex_res->get_height(), format, vk::Image::UsageField{vk::Image::Usage::TransferDestination, vk::Image::Usage::Sampleable}, vk::hardware::MemoryResidency::GPU};
            vk::ImageView view{*this->device, img};
            vk::Sampler img_sampler{*this->device};
            this->texture_resource_textures.push_back({std::move(img), std::move(view), std::move(img_sampler)});
        }
    }

    void RendererImageManagerVulkan::setup_depth_image()
    {
        auto swapchain_width = static_cast<std::uint32_t>(this->swapchain->get_width());
        auto swapchain_height = static_cast<std::uint32_t>(this->swapchain->get_height());
        this->depth_image = vk::Image{*this->device, swapchain_width, swapchain_height, vk::Image::Format::DepthFloat32, {vk::Image::Usage::DepthStencilAttachment}, vk::hardware::MemoryResidency::GPU};
        this->depth_imageview = vk::ImageView{*this->device, this->depth_image.value()};
    }

    void RendererImageManagerVulkan::setup_swapchain_framebuffers()
    {
        this->swapchain_framebuffers.clear();
        auto swapchain_width = static_cast<std::uint32_t>(this->swapchain->get_width());
        auto swapchain_height = static_cast<std::uint32_t>(this->swapchain->get_height());
        if(vk::is_headless())
        {
            if(this->depth_imageview.has_value())
            {
                this->swapchain_framebuffers.emplace_back(this->render_pass->vk_get_render_pass(), this->maybe_swapchain_offscreen_imageview.value(), this->depth_imageview.value(), VkExtent2D(swapchain_width, swapchain_height));
            }
            else
            {
                this->swapchain_framebuffers.emplace_back(this->render_pass->vk_get_render_pass(), this->maybe_swapchain_offscreen_imageview.value(), VkExtent2D(swapchain_width, swapchain_height));
            }
        }
        else
        {
            const auto& as_swapchain = static_cast<const vk::Swapchain&>(*this->swapchain);
            for(const vk::ImageView& swapchain_view : as_swapchain.get_image_views())
            {
                // If we have a depth image attached, add it to the framebuffer.
                if(this->depth_imageview.has_value())
                {
                    this->swapchain_framebuffers.emplace_back(this->render_pass->vk_get_render_pass(), swapchain_view, this->depth_imageview.value(), VkExtent2D{swapchain_width, swapchain_height});
                }
                else
                {
                    this->swapchain_framebuffers.emplace_back(this->render_pass->vk_get_render_pass(), swapchain_view, VkExtent2D{swapchain_width, swapchain_height});
                }
            }
        }
    }

    std::span<const vk::Framebuffer> RendererImageManagerVulkan::get_swapchain_framebuffers() const
    {
        return this->swapchain_framebuffers;
    }

    std::span<const IResource* const> RendererImageManagerVulkan::get_texture_resources() const
    {
        return this->texture_resources;
    }

    std::span<IResource*> RendererImageManagerVulkan::get_texture_resources()
    {
        return this->texture_resources;
    }

    std::span<const RendererImageManagerVulkan::TextureComponent> RendererImageManagerVulkan::get_resource_textures() const
    {
        return this->texture_resource_textures;
    }

    std::span<RendererImageManagerVulkan::TextureComponent> RendererImageManagerVulkan::get_resource_textures()
    {
        return this->texture_resource_textures;
    }

    RendererProcessorVulkan::RendererProcessorVulkan(RendererBuilderVulkan builder, RendererBuilderDeviceInfoVulkan device_info, const IRendererInput* input):
    device(device_info.device),
    physical_device(this->device->get_queue_family().dev),
    render_pass(&builder.get_render_pass()),
    swapchain(device_info.device_swapchain),
    input(input),
    resource_descriptor_pool(std::nullopt),
    command_pool(*this->device, this->device->get_queue_family(), vk::CommandPool::RecycleBuffer),
    graphics_present_queue(this->device->get_hardware_queue()),
    frame_admin(*this->device, RendererVulkan::frames_in_flight)
    {
        // Now the command pool
        this->initialise_command_pool();
    }

    void RendererProcessorVulkan::initialise_resource_descriptors(const RendererPipelineManagerVulkan& pipeline_manager, const RendererBufferManagerVulkan& buffer_manager, const RendererImageManagerVulkan& image_manager, std::vector<const IResource*> resources)
    {
        const vk::DescriptorSetLayout& layout = pipeline_manager.get_resource_descriptor_layout();
        if(!resources.empty())
        {
            // First use the layout and all resources to create the pool.
            vk::DescriptorPoolBuilder pool_builder;
            std::size_t view_count = this->get_view_count();
            pool_builder.with_capacity(view_count * resources.size());
            auto buffer_resources = resources | std::views::filter([](const IResource* const resource)
            {
                return resource->get_type() == ResourceType::Buffer;
            });
            auto texture_resources = resources | std::views::filter([](const IResource* const resource)
            {
                return resource->get_type() == ResourceType::Texture;
            });

            auto num_buffer_resources = std::ranges::distance(buffer_resources.begin(), buffer_resources.end());
            auto num_texture_resources = std::ranges::distance(texture_resources.begin(), texture_resources.end());

            auto image_count = view_count;
            
            for(decltype(num_buffer_resources) i = 0; i < num_buffer_resources; i++)
            {
                pool_builder.with_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, image_count);
            }

            for(decltype(num_texture_resources) i = 0; i < num_texture_resources; i++)
            {
                pool_builder.with_size(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, image_count);
            }
            for(std::size_t i = 0; i < image_count; i++)
            {
                pool_builder.with_layout(layout);
            }

            this->resource_descriptor_pool = {*this->device, pool_builder};
            // Now create the descriptor sets.
            vk::DescriptorSetsCreationRequests requests;
            for(std::size_t i = 0; i < image_count; i++)
            {
                vk::DescriptorSetsCreationRequest& request = requests.new_request();
                for(decltype(num_buffer_resources) j = 0; j < num_buffer_resources; j++)
                {
                    const vk::Buffer& resource_buffer = buffer_manager.get_resource_buffers()[j];
                    request.add_buffer(resource_buffer, 0, VK_WHOLE_SIZE, j);
                }
                for(decltype(num_texture_resources) j = 0; j < num_texture_resources; j++)
                {
                    const RendererImageManagerVulkan::TextureComponent& texture_component = image_manager.get_resource_textures()[j];
                    request.add_image(texture_component.view, texture_component.sampler, j + num_buffer_resources);
                }
            }
            this->resource_descriptor_pool->initialise_sets(requests);
        }
    }

    void RendererProcessorVulkan::initialise_command_pool()
    {
        if(!this->command_pool.empty())
        {
            this->command_pool.clear();
        }
        constexpr std::size_t num_scratch_command_bufs = 1;
        this->command_pool.with(this->get_view_count() + num_scratch_command_bufs);
    }

    void RendererProcessorVulkan::block_until_idle()
    {
        this->device->block_until_idle();
    }

    void RendererProcessorVulkan::record_rendering_commands(const RendererPipelineManagerVulkan& pipeline_manager, const RendererBufferManagerVulkan& buffer_manager, const RendererImageManagerVulkan& image_manager, tz::Vec4 clear_colour)
    {
        VkClearValue vk_clear_colour{clear_colour[0], clear_colour[1], clear_colour[2], clear_colour[3]};
        for(std::size_t i = 0; i < this->get_view_count(); i++)
        {
            vk::CommandBufferRecording render = this->command_pool[i].record();
            vk::RenderPassRun run{this->command_pool[i], this->render_pass->vk_get_render_pass(), image_manager.get_swapchain_framebuffers()[i], this->swapchain->full_render_area(), vk_clear_colour};
            pipeline_manager.get_pipeline().bind(this->command_pool[i]);
            if(buffer_manager.get_vertex_buffer() != nullptr)
            {
                render.bind(*buffer_manager.get_vertex_buffer());
            }
            if(buffer_manager.get_index_buffer() != nullptr)
            {
                render.bind(*buffer_manager.get_index_buffer());
            }
            if(this->resource_descriptor_pool.has_value())
            {
                render.bind(this->resource_descriptor_pool.value()[i], pipeline_manager.get_layout());
            }
            if(this->input != nullptr)
            {
                render.draw_indexed(this->input->get_indices().size());
            }
            else
            {
                // TODO: Specify custom number of vertices when there is no input?
                render.draw(0);
            }
        }
    }

    void RendererProcessorVulkan::record_and_run_scratch_commands(RendererBufferManagerVulkan& buffer_manager, RendererImageManagerVulkan& image_manager)
    {
        vk::Fence copy_fence{*this->device};
        copy_fence.signal();
        vk::CommandBuffer& scratch_buf = this->command_pool[this->get_view_count()];
        vk::Submit do_scratch_operation{vk::CommandBuffers{scratch_buf}, vk::SemaphoreRefs{}, vk::WaitStages{}, vk::SemaphoreRefs{}};

        if(this->input != nullptr && this->input->data_access() == RendererInputDataAccess::StaticFixed)
        {
            // Setup transfers using the scratch buffers.
            {
                // Part 1: Transfer vertex data.
                auto vertex_data = this->input->get_vertex_bytes();
                tz_report("VB (%zu vertices, %zu bytes total)", vertex_data.size(), vertex_data.size_bytes());
                vk::Buffer vertices_staging{vk::BufferType::Staging, vk::BufferPurpose::TransferSource, *this->device, vk::hardware::MemoryResidency::CPU, vertex_data.size_bytes()};
                vertices_staging.write(vertex_data.data(), vertex_data.size_bytes());
                {
                    vk::CommandBufferRecording transfer_vertices = scratch_buf.record();
                    transfer_vertices.buffer_copy_buffer(vertices_staging, *buffer_manager.get_vertex_buffer(), vertex_data.size_bytes());
                }

                // Submit Part 1
                do_scratch_operation(this->graphics_present_queue, copy_fence);

                copy_fence.wait_for();
                scratch_buf.reset();
                // Part 2: Transfer index data.
                auto index_data = this->input->get_indices();
                tz_report("IB (%zu indices, %zu bytes total)", index_data.size(), index_data.size_bytes());
                vk::Buffer indices_staging{vk::BufferType::Staging, vk::BufferPurpose::TransferSource, *this->device, vk::hardware::MemoryResidency::CPU, index_data.size_bytes()};
                indices_staging.write(index_data.data(), index_data.size_bytes());
                {
                    vk::CommandBufferRecording transfer_indices = scratch_buf.record();
                    transfer_indices.buffer_copy_buffer(indices_staging, *buffer_manager.get_index_buffer(), index_data.size_bytes());
                }
                copy_fence.signal();
                // Submit Part 2
                do_scratch_operation(this->graphics_present_queue, copy_fence);
                copy_fence.wait_for();
            }
        }

        tz_assert(buffer_manager.get_resource_buffers().size() == buffer_manager.get_buffer_resources().size(), "Resource buffer size != Buffer resource size");
        for(std::size_t i = 0; i < buffer_manager.get_resource_buffers().size(); i++)
        {
            vk::Buffer& resource_buffer = buffer_manager.get_resource_buffers()[i];
            IResource* buffer_resource = buffer_manager.get_buffer_resources()[i];
            tz_report("Buffer Resource (ResourceID: %zu, BufferComponentID: %zu, %zu bytes total)", i, i, buffer_resource->get_resource_bytes().size_bytes());
            if(buffer_resource->data_access() == RendererInputDataAccess::StaticFixed)
            {
                // Do transfers now.
                scratch_buf.reset();
                vk::Buffer resource_staging{vk::BufferType::Staging, vk::BufferPurpose::TransferSource, *this->device, vk::hardware::MemoryResidency::CPU, buffer_resource->get_resource_bytes().size_bytes()};
                resource_staging.write(buffer_resource->get_resource_bytes().data(), buffer_resource->get_resource_bytes().size_bytes());
                {
                    vk::CommandBufferRecording transfer_resource = scratch_buf.record();
                    transfer_resource.buffer_copy_buffer(resource_staging, resource_buffer, buffer_resource->get_resource_bytes().size_bytes());
                }
                copy_fence.signal();
                do_scratch_operation(this->graphics_present_queue, copy_fence);
                copy_fence.wait_for();
            }
        }

        for(std::size_t i = 0; i < image_manager.get_resource_textures().size(); i++)
        {
            RendererImageManagerVulkan::TextureComponent& texture_component = image_manager.get_resource_textures()[i];
            IResource* texture_resource = image_manager.get_texture_resources()[i];
            tz_report("Texture Resource (ResourceID: %zu, TextureComponentID: %zu, %zu bytes total)", buffer_manager.get_buffer_resources().size() + i, i, texture_resource->get_resource_bytes().size_bytes());
            tz_assert(texture_resource->data_access() == RendererInputDataAccess::StaticFixed, "DynamicFixed texture resources not yet implemented (Vulkan)");
            {
                scratch_buf.reset();
                vk::Buffer resource_staging{vk::BufferType::Staging, vk::BufferPurpose::TransferSource, *this->device, vk::hardware::MemoryResidency::CPU, texture_resource->get_resource_bytes().size_bytes()};
                resource_staging.write(texture_resource->get_resource_bytes().data(), texture_resource->get_resource_bytes().size_bytes());
                {
                    vk::CommandBufferRecording transfer_image = scratch_buf.record();
                    transfer_image.transition_image_layout(texture_component.img, vk::Image::Layout::TransferDestination);
                    transfer_image.buffer_copy_image(resource_staging, texture_component.img);
                    transfer_image.transition_image_layout(texture_component.img, vk::Image::Layout::ShaderResource);
                }
                copy_fence.signal();
                do_scratch_operation(this->graphics_present_queue, copy_fence);
                copy_fence.wait_for();
            }
        }
    }

    void RendererProcessorVulkan::set_regeneration_function(std::function<void()> action)
    {
        this->frame_admin.set_regeneration_function(action);
    }

    void RendererProcessorVulkan::render()
    {
        if(vk::is_headless())
        {
            this->frame_admin.render_frame_headless(this->graphics_present_queue, 1, this->command_pool, vk::WaitStages{vk::WaitStage::ColourAttachmentOutput});
        }
        else
        {
            this->frame_admin.render_frame(this->graphics_present_queue, static_cast<const vk::Swapchain&>(*this->swapchain), this->command_pool, vk::WaitStages{vk::WaitStage::ColourAttachmentOutput});
        }
    }

    std::size_t RendererProcessorVulkan::get_view_count() const
    {
        if(vk::is_headless())
        {
            return 1;
        }
        else
        {
            return static_cast<const vk::Swapchain&>(*this->swapchain).get_image_views().size();
        }
    }

    RendererVulkan::RendererVulkan(RendererBuilderVulkan builder, RendererBuilderDeviceInfoVulkan device_info):
    renderer_input(builder.get_input() == nullptr ? nullptr : builder.get_input()->unique_clone()),
    renderer_resources(),
    buffer_manager(device_info, this->renderer_input.get()),
    pipeline_manager(builder, device_info),
    image_manager(builder, device_info),
    processor(builder, device_info, this->renderer_input.get()),
    clear_colour(),
    requires_depth_image(builder.get_render_pass().requires_depth_image())
    {
        this->clear_colour = {0.0f, 0.0f, 0.0f, 0.0f};

        std::vector<const IResource*> all_resources;
        std::vector<IResource*> buffer_resources;
        std::vector<IResource*> texture_resources;
        for(const IResource* buffer_resource : builder.vk_get_buffer_resources())
        {
            this->renderer_resources.push_back(buffer_resource->unique_clone());
            IResource* new_resource = this->renderer_resources.back().get();
            buffer_resources.push_back(new_resource);
            all_resources.push_back(new_resource);
        }
        for(const IResource* texture_resource : builder.vk_get_texture_resources())
        {
            this->renderer_resources.push_back(texture_resource->unique_clone());
            IResource* new_resource = this->renderer_resources.back().get();
            texture_resources.push_back(new_resource);
            all_resources.push_back(new_resource);
        }

        this->buffer_manager.initialise_resources(buffer_resources);
        this->buffer_manager.setup_buffers();

        this->image_manager.initialise_resources(texture_resources);
        if(this->requires_depth_image)
        {
            this->image_manager.setup_depth_image();
        }
        this->image_manager.setup_swapchain_framebuffers();

        this->processor.initialise_resource_descriptors(this->pipeline_manager, this->buffer_manager, this->image_manager, all_resources);
        // Command Buffers for each swapchain image, but an extra general-purpose recycleable buffer.
        // Now setup the swapchain image buffers
        this->processor.record_rendering_commands(this->pipeline_manager, this->buffer_manager, this->image_manager, this->clear_colour);

        this->processor.record_and_run_scratch_commands(this->buffer_manager, this->image_manager);
        // If frame admin needs to regenerate, allow it to.
        this->processor.set_regeneration_function([this](){this->handle_resize();});
        // Tell the device to notify us when it detects a window resize. We will also need to regenerate then too.
        *device_info.on_resize = [this](){this->handle_resize();};
        tz_report("RendererVulkan (%s, %zu resource%s)", this->renderer_input != nullptr ? "Input" : "No Input", this->renderer_resources.size(), this->renderer_resources.size() == 1 ? "" : "s");
    }

    void RendererVulkan::set_clear_colour(tz::Vec4 clear_colour)
    {
        this->clear_colour = clear_colour;
        this->handle_clear_colour_change();
    }

    tz::Vec4 RendererVulkan::get_clear_colour() const
    {
        return this->clear_colour;
    }

    IRendererInput* RendererVulkan::get_input()
    {
        return this->renderer_input.get();
    }

    IResource* RendererVulkan::get_resource(ResourceHandle handle)
    {
        auto handle_value = static_cast<HandleValueUnderlying>(static_cast<HandleValue>(handle));
        return this->renderer_resources[handle_value].get();
    }

    void RendererVulkan::render()
    {
        this->processor.render();
    }

    void RendererVulkan::handle_resize()
    {
        this->pipeline_manager.reconstruct_pipeline();

        if(this->requires_depth_image)
        {
            this->image_manager.setup_depth_image();
        }

        this->image_manager.setup_swapchain_framebuffers();

        this->processor.initialise_command_pool();
        this->processor.record_rendering_commands(this->pipeline_manager, this->buffer_manager, this->image_manager, this->clear_colour);
    }

    void RendererVulkan::handle_clear_colour_change()
    {
        this->processor.block_until_idle();

        this->processor.initialise_command_pool();
        this->processor.record_rendering_commands(this->pipeline_manager, this->buffer_manager, this->image_manager, this->clear_colour);
    }
}
#endif // TZ_VULKAN