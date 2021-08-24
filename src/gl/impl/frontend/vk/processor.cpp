#if TZ_VULKAN
#include "core/profiling/zone.hpp"
#include "gl/impl/frontend/vk/processor.hpp"
#include "gl/impl/backend/vk/submit.hpp"
#include "gl/resource.hpp"
#include "gl/api/renderer.hpp"

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

    std::span<const IResource* const> ProcessorBuilderVulkan::vk_get_buffer_resources() const
    {
        return this->buffer_resources;
    }

    std::span<const IResource* const> ProcessorBuilderVulkan::vk_get_texture_resources() const
    {
        return this->texture_resources;
    }

    ProcessorResourceManagerVulkan::ProcessorResourceManagerVulkan(ProcessorBuilderVulkan builder, ProcessorDeviceInfoVulkan device_info):
    device(device_info.vk_device)
    {
        for(const IResource* const buf_res : builder.vk_get_buffer_resources())
        {
            this->buffer_resources.push_back(buf_res->unique_clone());
        }
        for(const IResource* const tex_res : builder.vk_get_texture_resources())
        {
            this->texture_resources.push_back(tex_res->unique_clone());
        }
        this->setup_buffers();
        this->setup_textures();
    }

    std::size_t ProcessorResourceManagerVulkan::resource_count() const
    {
        return this->buffer_components.size() + this->texture_components.size();
    }

    std::size_t ProcessorResourceManagerVulkan::resource_count_of(ResourceType type) const
    {
        switch(type)
        {
            case ResourceType::Buffer:
                return this->buffer_components.size();
            break;
            case ResourceType::Texture:
                return this->texture_components.size();
            break;
            default:
                tz_error("Unknown ResourceType (Vulkan)");
                return 0;
            break;
        }
    }

    IResource* ProcessorResourceManagerVulkan::get_resource(ResourceHandle handle)
    {
        auto handle_value = static_cast<HandleValueUnderlying>(static_cast<HandleValue>(handle));

        if(handle_value < this->buffer_components.size())
        {
            return this->buffer_components[handle_value].get_resource();
        }
        else if(handle_value < this->buffer_components.size() + this->texture_components.size())
        {
            return this->texture_components[handle_value - this->buffer_components.size()].get_resource();
        }
        else
        {
            tz_error("Invalid ResourceHandle");
            return nullptr;
        }
    }

    void ProcessorResourceManagerVulkan::setup_buffers()
    {
        for(const auto& buf_res_ptr : this->buffer_resources)
        {
            this->buffer_components.emplace_back(buf_res_ptr.get());
        }

        for(BufferComponentVulkan& buffer_component : this->buffer_components)
        {
            IResource* buffer_resource = buffer_component.get_resource();
            switch(buffer_resource->data_access())
            {
                case RendererInputDataAccess::StaticFixed:
                    buffer_component.set_buffer(vk::Buffer{vk::BufferType::Uniform, vk::BufferPurpose::TransferDestination, *this->device, vk::hardware::MemoryResidency::GPU, buffer_resource->get_resource_bytes().size_bytes()});
                break;
                case RendererInputDataAccess::DynamicFixed:
                    {
                        auto& dynamic_resource = static_cast<IDynamicResource&>(*buffer_resource);
                        buffer_component.set_buffer(vk::Buffer{vk::BufferType::Uniform, vk::BufferPurpose::NothingSpecial, *this->device, vk::hardware::MemoryResidency::CPUPersistent, dynamic_resource.get_resource_bytes().size_bytes()});
                        dynamic_resource.set_resource_data(static_cast<std::byte*>(buffer_component.get_buffer().map_memory()));
                    }
                break;
                default:
                    tz_error("Resource data access unsupported (Vulkan)");
                break;
            }
        }
    }

    void ProcessorResourceManagerVulkan::setup_textures()
    {
        for(const auto& texture_resource_ptr : this->texture_resources)
        {
            IResource* texture_resource = texture_resource_ptr.get();
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
                case TextureFormat::Bgra32UnsignedNorm:
                    format = vk::Image::Format::Bgra32UnsignedNorm;
                break;
                default:
                    tz_error("Unrecognised texture format (Vulkan)");
                break;
            }

            vk::SamplerProperties props;
            {
                TextureProperties gl_props = tex_res->get_properties();
                auto convert_filter = [](TexturePropertyFilter filter)
                {
                    switch(filter)
                    {
                        case TexturePropertyFilter::Nearest:
                            return VK_FILTER_NEAREST;
                        break;
                        case TexturePropertyFilter::Linear:
                            return VK_FILTER_LINEAR;
                        break;
                        default:
                            tz_error("Vulkan support for TexturePropertyFilter is not yet implemented");
                            return static_cast<VkFilter>(0);
                        break;
                    }
                };
                auto convert_address_mode = [](TextureAddressMode addr_mode)
                {
                    switch(addr_mode)
                    {
                        case TextureAddressMode::ClampToEdge:
                            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                        break;
                        default:
                            tz_error("Vulkan support for TextureAddressMode is not yet implemented");
                            return static_cast<VkSamplerAddressMode>(0);
                        break;
                    }
                };
                props.min_filter = convert_filter(gl_props.min_filter);
                props.mag_filter = convert_filter(gl_props.mag_filter);

                props.address_mode_u = convert_address_mode(gl_props.address_mode_u);
                props.address_mode_v = convert_address_mode(gl_props.address_mode_v);
                props.address_mode_w = convert_address_mode(gl_props.address_mode_w);
            }

            vk::Image img{*this->device, tex_res->get_width(), tex_res->get_height(), format, vk::Image::UsageField{vk::Image::Usage::TransferDestination, vk::Image::Usage::Sampleable, vk::Image::Usage::ColourAttachment}, vk::hardware::MemoryResidency::GPU};
            vk::ImageView view{*this->device, img};
            vk::Sampler img_sampler{*this->device, props};
            this->texture_components.emplace_back(texture_resource, std::move(img), std::move(view), std::move(img_sampler));
        }
    }

    ProcessorVulkan::ProcessorVulkan(ProcessorBuilderVulkan builder, ProcessorDeviceInfoVulkan device_info):
    descriptor_layout(builder.vk_get_descriptor_set_layout(*device_info.vk_device)),
    compute_pipeline
    (
        vk::pipeline::ShaderStage{builder.vk_get_compute_shader(), vk::pipeline::ShaderType::Compute},
        *device_info.vk_device,
        vk::pipeline::Layout{*device_info.vk_device, vk::DescriptorSetLayoutRefs{this->descriptor_layout}}
    ),
    resource_manager(builder, device_info),
    resource_descriptor_pool(std::nullopt),
    command_pool(*device_info.vk_device, device_info.vk_device->get_queue_family(), vk::CommandPool::RecycleBuffer),
    compute_queue(device_info.vk_device->get_hardware_queue()),
    process_fence(*device_info.vk_device)
    {
        this->command_pool.with(1);
        this->record_processing_commands();
    }

    std::size_t ProcessorVulkan::resource_count() const
    {
        return this->resource_manager.resource_count();
    }

    std::size_t ProcessorVulkan::resource_count_of(ResourceType type) const
    {
        return this->resource_manager.resource_count_of(type);
    }

    IResource* ProcessorVulkan::get_resource(ResourceHandle handle)
    {
        return this->resource_manager.get_resource(handle);
    }

    void ProcessorVulkan::process()
    {
        //std::printf("ayy lmao\n");
        vk::Submit submit{vk::CommandBuffers{command_pool[0]}, vk::SemaphoreRefs{}, vk::WaitStages{vk::WaitStage::Compute}, vk::SemaphoreRefs{}};
        this->process_fence.wait_then_signal();
        submit(this->compute_queue, this->process_fence);
        this->process_fence.wait_for();
    }

    void ProcessorVulkan::record_processing_commands()
    {
        TZ_PROFZONE("Record Processing Commands", TZ_PROFCOL_YELLOW);
        {
            vk::CommandBuffer& cmd_buf = this->command_pool[0];
            vk::CommandBufferRecording render = cmd_buf.record();
            this->compute_pipeline.bind(cmd_buf);
            render.dispatch(1, 1, 1);
        }
        /*
TZ_PROFZONE("Record Rendering Commands", TZ_PROFCOL_YELLOW);
        VkClearValue vk_clear_colour{clear_colour[0], clear_colour[1], clear_colour[2], clear_colour[3]};
        for(std::size_t i = 0; i < this->get_view_count(); i++)
        {
            vk::CommandBufferRecording render = this->command_pool[i].record();
            const vk::Framebuffer* render_target = nullptr;
            if(image_manager.get_texture_output() != nullptr)
            {
                render_target = image_manager.get_texture_output();
            } 
            else
            {
                render_target = &image_manager.get_swapchain_framebuffers()[i];
            }
            vk::RenderPassRun run{this->command_pool[i], this->render_pass->vk_get_render_pass(), *render_target, this->swapchain->full_render_area(), vk_clear_colour};
            pipeline_manager.get_pipeline().bind(this->command_pool[i]);
            if(this->resource_descriptor_pool.has_value())
            {
                render.bind(this->resource_descriptor_pool.value()[i], pipeline_manager.get_layout());
            }
            if(this->inputs.empty())
            {
                render.draw(3);
            }
            else
            {
                if(this->draw_indirect_buffer.has_value())
                {
                    render.bind(buffer_manager.get_vertex_buffer());
                    render.bind(buffer_manager.get_index_buffer());
                    render.draw_indirect(this->draw_indirect_buffer.value(), this->num_static_draws());
                }
                if(this->draw_indirect_dynamic_buffer.has_value())
                {
                    render.bind(buffer_manager.get_dynamic_vertex_buffer());
                    render.bind(buffer_manager.get_dynamic_index_buffer());
                    render.draw_indirect(this->draw_indirect_dynamic_buffer.value(), this->num_dynamic_draws());
                }
            }
        }
        */
    }
}

#endif // TZ_VULKAN