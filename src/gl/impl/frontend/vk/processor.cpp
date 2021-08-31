#if TZ_VULKAN
#include "core/profiling/zone.hpp"
#include "core/report.hpp"
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
            vk::DescriptorType desc_type;
            const tz::gl::ShaderMeta& meta = this->shader->get_meta();
            ShaderMetaValue value = meta.try_get_meta_value(i).value_or(ShaderMetaValue::UBO);
            switch(value)
            {
                case ShaderMetaValue::UBO:
                    desc_type = vk::DescriptorType::UniformBuffer;
                break;
                case ShaderMetaValue::SSBO:
                    desc_type = vk::DescriptorType::ShaderStorageBuffer;
                break;
                default:
                {
                    const char* meta_value_name = detail::meta_value_names[static_cast<int>(value)];
                    tz_error("Unexpected Shader meta value. Expecting a buffer-y meta value, but instead got \"%s\"", meta_value_name);
                }
                break;
            }
            layout_builder.add(desc_type, {vk::pipeline::ShaderType::Compute});
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

    ProcessorResourceManagerVulkan::ProcessorResourceManagerVulkan(ProcessorBuilderVulkan builder, ProcessorDeviceInfoVulkan device_info, const vk::DescriptorSetLayout& descriptor_set_layout, const vk::pipeline::Layout& pipeline_layout):
    device(device_info.vk_device),
    descriptor_layout(&descriptor_set_layout),
    pipeline_layout(&pipeline_layout),
    shader(&builder.get_shader())
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
        this->initialise_resource_descriptors();
    }

    std::size_t ProcessorResourceManagerVulkan::resource_count() const
    {
        return this->components.component_count();
    }

    std::size_t ProcessorResourceManagerVulkan::resource_count_of(ResourceType type) const
    {
        return this->components.component_count_of(type);
    }

    IResource* ProcessorResourceManagerVulkan::get_resource(ResourceHandle handle)
    {
        return this->components.get_component(handle)->get_resource();
    }

    const vk::DescriptorPool* ProcessorResourceManagerVulkan::get_descriptor_pool() const
    {
        if(this->resource_descriptor_pool.has_value())
        {
            return &this->resource_descriptor_pool.value();
        }
        return nullptr;
    }

    BufferComponentVulkan* ProcessorResourceManagerVulkan::get_buffer_component(std::size_t buffer_id)
    {
        std::size_t res_id = buffer_id;
        return static_cast<BufferComponentVulkan*>(this->components.get_component(tz::HandleValue{res_id}));
    }

    TextureComponentVulkan* ProcessorResourceManagerVulkan::get_texture_component(std::size_t texture_id)
    {
        std::size_t res_id = texture_id + this->resource_count_of(ResourceType::Buffer);
        return static_cast<TextureComponentVulkan*>(this->components.get_component(tz::HandleValue{res_id}));
    }

    void ProcessorResourceManagerVulkan::setup_buffers()
    {
        for(std::size_t i = 0; i < this->buffer_resources.size(); i++)
        {
            IResource* buf_res = this->buffer_resources[i].get();
            // For each buffer resource, generate a buffercomponent.
            ResourceHandle comp_handle = this->components.emplace_component<BufferComponentVulkan>(buf_res);
            auto* buffer_component = static_cast<BufferComponentVulkan*>(this->components.get_component(comp_handle));

            vk::BufferType buf_type;
            const tz::gl::ShaderMeta& meta = this->shader->get_meta();
            ShaderMetaValue value = meta.try_get_meta_value(i).value_or(ShaderMetaValue::UBO);
            switch(value)
            {
                case ShaderMetaValue::UBO:
                    buf_type = vk::BufferType::Uniform;
                break;
                case ShaderMetaValue::SSBO:
                    buf_type = vk::BufferType::ShaderStorage;
                break;
                default:
                {
                    const char* meta_value_name = detail::meta_value_names[static_cast<int>(value)];
                    tz_error("Unexpected Shader meta value. Expecting a buffer-y meta value, but instead got \"%s\"", meta_value_name);
                }
                break;
            }

            IResource* buffer_resource = buffer_component->get_resource();
            switch(buffer_resource->data_access())
            {
                case RendererInputDataAccess::StaticFixed:

                    buffer_component->set_buffer(vk::Buffer{buf_type, vk::BufferPurpose::TransferDestination, *this->device, vk::hardware::MemoryResidency::GPU, buffer_resource->get_resource_bytes().size_bytes()});
                break;
                case RendererInputDataAccess::DynamicFixed:
                    {
                        auto& dynamic_resource = static_cast<IDynamicResource&>(*buffer_resource);
                        buffer_component->set_buffer(vk::Buffer{buf_type, vk::BufferPurpose::NothingSpecial, *this->device, vk::hardware::MemoryResidency::CPUPersistent, dynamic_resource.get_resource_bytes().size_bytes()});
                        dynamic_resource.set_resource_data(static_cast<std::byte*>(buffer_component->get_buffer().map_memory()));
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
            this->components.emplace_component<TextureComponentVulkan>(texture_resource, std::move(img), std::move(view), std::move(img_sampler));
            //this->texture_components.emplace_back(texture_resource, std::move(img), std::move(view), std::move(img_sampler));
        }
    }

    void ProcessorResourceManagerVulkan::initialise_resource_descriptors()
    {
        const vk::DescriptorSetLayout& layout = *this->descriptor_layout;
        if(this->resource_count() > 0)
        {
            // First use the layout and all resources to create the pool.
            vk::DescriptorPoolBuilder pool_builder;
            std::size_t view_count = 1;
            pool_builder.with_capacity(view_count * this->resource_count());

            for(std::size_t i = 0; i < this->resource_count_of(ResourceType::Buffer); i++)
            {
                vk::BufferType buf_type = this->get_buffer_component(i)->get_buffer().get_type();
                VkDescriptorType desc_type;
                switch(buf_type)
                {
                    case vk::BufferType::Uniform:
                        desc_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    break;
                    case vk::BufferType::ShaderStorage:
                        desc_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    break;
                    default:
                        tz_error("Provided BufferType is not valid for a BufferResource");
                    break;
                }
                pool_builder.with_size(desc_type, view_count);
            }

            for(std::size_t i = 0; i < this->resource_count_of(ResourceType::Texture); i++)
            {
                pool_builder.with_size(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, view_count);
            }

            for(std::size_t i = 0; i < view_count; i++)
            {
                pool_builder.with_layout(layout);
            }

            this->resource_descriptor_pool = {*this->device, pool_builder};
            // Now create the descriptor sets.
            vk::DescriptorSetsCreationRequests requests;
            for(std::size_t i = 0; i < view_count; i++)
            {
                vk::DescriptorSetsCreationRequest& request = requests.new_request();
                for(std::size_t j = 0; j < this->resource_count_of(ResourceType::Buffer); j++)
                {
                    const vk::Buffer& resource_buffer = this->get_buffer_component(j)->get_buffer();
                    request.add_buffer(resource_buffer, 0, VK_WHOLE_SIZE, j);
                }

                for(std::size_t j = 0; j < this->resource_count_of(ResourceType::Texture); j++)
                {
                    const TextureComponentVulkan& texture_component = *this->get_texture_component(j);
                    request.add_image(texture_component.get_view(), texture_component.get_sampler(), j + this->resource_count_of(ResourceType::Buffer));
                }
            }
            this->resource_descriptor_pool->initialise_sets(requests);
        }
    }

    ProcessorVulkan::ProcessorVulkan(ProcessorBuilderVulkan builder, ProcessorDeviceInfoVulkan device_info):
    device(device_info.vk_device),
    descriptor_layout(builder.vk_get_descriptor_set_layout(*this->device)),
    pipeline_layout{*this->device, vk::DescriptorSetLayoutRefs{this->descriptor_layout}},
    compute_pipeline
    (
        vk::pipeline::ShaderStage{builder.vk_get_compute_shader(), vk::pipeline::ShaderType::Compute},
        *device_info.vk_device,
        this->pipeline_layout
    ),
    resource_manager(builder, device_info, this->descriptor_layout, this->pipeline_layout),
    command_pool(*this->device, this->device->get_queue_family(), vk::CommandPool::RecycleBuffer),
    compute_queue(this->device->get_hardware_queue()),
    process_fence(*this->device)
    {
        this->command_pool.with(2);
        this->record_processing_commands();
        this->record_and_run_scratch_commands();
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
            if(this->resource_manager.get_descriptor_pool() != nullptr)
            {
                const vk::DescriptorPool& pool = *this->resource_manager.get_descriptor_pool();
                render.bind_compute(pool[0], this->pipeline_layout);
            }
            render.dispatch(1, 1, 1);
        }
    }

    void ProcessorVulkan::record_and_run_scratch_commands()
    {
        TZ_PROFZONE("Scratch : Record and Run Scratch Commands", TZ_PROFCOL_YELLOW);
        vk::Fence copy_fence{*this->device};
        copy_fence.signal();
        vk::CommandBuffer& scratch_buf = this->command_pool[1];
        vk::Submit do_scratch_operation{vk::CommandBuffers{scratch_buf}, vk::SemaphoreRefs{}, vk::WaitStages{}, vk::SemaphoreRefs{}};
        // Run scratch commands to ensure resource data is initialised properly.
        {
            // Setup all components for buffer resources.
            TZ_PROFZONE("Scratch : Buffer Resources", TZ_PROFCOL_YELLOW);
            for(std::size_t i = 0; i < this->resource_manager.resource_count_of(ResourceType::Buffer); i++)
            {
                BufferComponentVulkan& buffer_component = *this->resource_manager.get_buffer_component(i);
                vk::Buffer& resource_buffer = buffer_component.get_buffer();
                IResource* buffer_resource = buffer_component.get_resource();
                tz_report("Buffer Resource (ResourceID: %zu, BufferComponentID: %zu, %zu bytes total)", i, i, buffer_resource->get_resource_bytes().size_bytes());
                if(buffer_resource->data_access() == RendererInputDataAccess::StaticFixed)
                {
                    TZ_PROFZONE("Scratch : Copy Buffer Data", TZ_PROFCOL_RED);
                    // Do transfers now.
                    scratch_buf.reset();
                    vk::Buffer resource_staging{vk::BufferType::Staging, vk::BufferPurpose::TransferSource, *this->device, vk::hardware::MemoryResidency::CPU, buffer_resource->get_resource_bytes().size_bytes()};
                    resource_staging.write(buffer_resource->get_resource_bytes().data(), buffer_resource->get_resource_bytes().size_bytes());
                    {
                        vk::CommandBufferRecording transfer_resource = scratch_buf.record();
                        transfer_resource.buffer_copy_buffer(resource_staging, resource_buffer, buffer_resource->get_resource_bytes().size_bytes());
                    }
                    copy_fence.signal();
                    do_scratch_operation(this->compute_queue, copy_fence);
                    copy_fence.wait_for();
                }
            }
        }

        {
            // Setup all components for texture resources.
            TZ_PROFZONE("Scratch : Texture Resources", TZ_PROFCOL_YELLOW);
            for(std::size_t i = 0; i < this->resource_manager.resource_count_of(ResourceType::Texture); i++)
            {
                TextureComponentVulkan& texture_component = *this->resource_manager.get_texture_component(i);
                IResource* texture_resource = texture_component.get_resource();
                tz_report("Texture Resource (ResourceID: %zu, TextureComponentID: %zu, %zu bytes total)", this->resource_manager.resource_count_of(ResourceType::Buffer) + i, i, texture_resource->get_resource_bytes().size_bytes());
                tz_assert(texture_resource->data_access() == RendererInputDataAccess::StaticFixed, "DynamicFixed texture resources not yet implemented (Vulkan)");
                {
                    TZ_PROFZONE("Scratch : Copy Image Data", TZ_PROFCOL_RED);
                    scratch_buf.reset();
                    vk::Buffer resource_staging{vk::BufferType::Staging, vk::BufferPurpose::TransferSource, *this->device, vk::hardware::MemoryResidency::CPU, texture_resource->get_resource_bytes().size_bytes()};
                    resource_staging.write(texture_resource->get_resource_bytes().data(), texture_resource->get_resource_bytes().size_bytes());
                    {
                        vk::CommandBufferRecording transfer_image = scratch_buf.record();
                        transfer_image.transition_image_layout(texture_component.get_image(), vk::Image::Layout::TransferDestination);
                        transfer_image.buffer_copy_image(resource_staging, texture_component.get_image());
                        transfer_image.transition_image_layout(texture_component.get_image(), vk::Image::Layout::ShaderResource);
                    }
                    copy_fence.signal();
                    do_scratch_operation(this->compute_queue, copy_fence);
                    copy_fence.wait_for();
                }
            }
        }
    }
}

#endif // TZ_VULKAN