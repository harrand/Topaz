#if TZ_VULKAN
#include "core/report.hpp"
#include "core/profiling/zone.hpp"
#include "gl/resource.hpp"
#include "gl/output.hpp"
#include "gl/impl/frontend/vk/renderer.hpp"
#include "gl/impl/frontend/vk/device.hpp"
#include "gl/impl/backend/vk/tz_vulkan.hpp"
#include "gl/impl/backend/vk/fence.hpp"
#include "gl/impl/backend/vk/submit.hpp"
#include <numeric>
#include <ranges>

namespace tz::gl
{
   using DrawIndirectCommand = VkDrawIndexedIndirectCommand;

    vk::pipeline::VertexInputState RendererBuilderVulkan::vk_get_vertex_input() const
    {
        if(this->inputs.empty())
        {
            // Just default vertexinput as we don't have any inputs.
            return {};
        }

        // Do the work to retrieve the formats and build up the vertex input state.
        RendererElementFormat fmt = this->inputs[0]->get_format();
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
            vk::DescriptorType desc_type = vk::DescriptorType::UniformBuffer;
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
            layout_builder.add(desc_type, vk::pipeline::ShaderTypeField::All());
        }
        for(std::size_t i = this->buffer_resources.size(); i < this->buffer_resources.size() + this->texture_resources.size(); i++)
        {
            layout_builder.add(vk::DescriptorType::CombinedImageSampler, vk::pipeline::ShaderTypeField::All());
        }
        return {device, layout_builder};
    }

    std::span<const IRendererInput* const> RendererBuilderVulkan::vk_get_inputs() const
    {
        return {this->inputs.begin(), this->inputs.end()};
    }

    std::span<const IResource* const> RendererBuilderVulkan::vk_get_buffer_resources() const
    {
        return {this->buffer_resources.begin(), this->buffer_resources.end()};
    }

    std::span<const IResource* const> RendererBuilderVulkan::vk_get_texture_resources() const
    {
        return {this->texture_resources.begin(), this->texture_resources.end()};
    }

    RendererPipelineManagerVulkan::RendererPipelineManagerVulkan(RendererBuilderVulkan builder, RendererBuilderDeviceInfoVulkan device_info, const RenderPassVulkan& render_pass):
    device(device_info.device),
    render_pass(&render_pass),
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

    RendererBufferManagerVulkan::RendererBufferManagerVulkan(RendererBuilderDeviceInfoVulkan device_info, std::vector<IRendererInput*> renderer_inputs, const Shader& shader):
    device(device_info.device),
    physical_device(this->device->get_queue_family().dev),
    shader(&shader),
    inputs(renderer_inputs),
    vertex_buffer(vk::Buffer::null()),
    dynamic_vertex_buffer(vk::Buffer::null()),
    index_buffer(vk::Buffer::null()),
    dynamic_index_buffer(vk::Buffer::null()),
    buffer_components()
    {
    }

    void RendererBufferManagerVulkan::initialise_resources(std::vector<IResource*> renderer_buffer_resources)
    {
        for(IResource* resource : renderer_buffer_resources)
        {
            this->buffer_components.emplace_back(resource);
        }
    }

    void RendererBufferManagerVulkan::setup_buffers()
    {
        struct DynamicInputMapRegion
        {
            IRendererDynamicInput& input;
            std::size_t offset;
            std::size_t length;
        };

        if(!this->inputs.empty())
        {
            std::vector<std::byte> static_vertex_bytes, dynamic_vertex_bytes;
            std::vector<unsigned int> static_indices, dynamic_indices;
            std::vector<DynamicInputMapRegion> vertex_regions, index_regions;

            bool any_static_geometry = false;
            bool any_dynamic_geometry = false;

            RendererElementFormat fmt;
            
            // Step 1: Compile all data
            for(IRendererInput* input : this->inputs)
            {
                fmt = input->get_format();
                std::span<const std::byte> input_vertices = input->get_vertex_bytes();
                std::span<const unsigned int> input_indices = input->get_indices();
                switch(input->data_access())
                {
                    case RendererInputDataAccess::StaticFixed:
                        std::copy(input_vertices.begin(), input_vertices.end(), std::back_inserter(static_vertex_bytes));
                        std::copy(input_indices.begin(), input_indices.end(), std::back_inserter(static_indices));
                        any_static_geometry = true;
                    break;
                    case RendererInputDataAccess::DynamicFixed:
                    {
                        std::size_t vertex_region_offset = dynamic_vertex_bytes.size();
                        std::size_t vertex_region_length = input_vertices.size_bytes();
                        std::copy(input_vertices.begin(), input_vertices.end(), std::back_inserter(dynamic_vertex_bytes));
                        std::size_t index_region_offset = dynamic_indices.size() * sizeof(unsigned int);
                        std::size_t index_region_length = input_indices.size_bytes();
                        std::copy(input_indices.begin(), input_indices.end(), std::back_inserter(dynamic_indices));

                        vertex_regions.push_back({.input = *static_cast<IRendererDynamicInput*>(input), .offset = vertex_region_offset, .length = vertex_region_length});
                        index_regions.push_back({.input = *static_cast<IRendererDynamicInput*>(input), .offset = index_region_offset, .length = index_region_length});
                        any_dynamic_geometry = true;
                    }
                    break;
                    default:
                        tz_error("Input data access unsupported (Vulkan)");
                    break;
                }
            }

            // Step 2: Fill buffers and map properly.
            if(any_static_geometry)
            {
                this->vertex_buffer = vk::Buffer{vk::BufferType::Vertex, vk::BufferPurpose::TransferDestination, *this->device, vk::hardware::MemoryResidency::GPU, static_vertex_bytes.size()};
                tz_report("VB Static (%zu vertices, %zu bytes total)", static_vertex_bytes.size() / fmt.binding_size, static_vertex_bytes.size());
                this->index_buffer = vk::Buffer{vk::BufferType::Index, vk::BufferPurpose::TransferDestination, *this->device, vk::hardware::MemoryResidency::GPU, static_indices.size() * sizeof(unsigned int)};
                tz_report("IB Static (%zu indices, %zu bytes total)", static_indices.size(), static_indices.size() * sizeof(unsigned int));
            }
            if(any_dynamic_geometry)
            {
                this->dynamic_vertex_buffer = vk::Buffer{vk::BufferType::Vertex, vk::BufferPurpose::NothingSpecial, *this->device, vk::hardware::MemoryResidency::CPUPersistent, dynamic_vertex_bytes.size()};
                tz_report("VB Dynamic (%zu vertices, %zu bytes total)", dynamic_vertex_bytes.size() / fmt.binding_size, dynamic_vertex_bytes.size());
                this->dynamic_index_buffer = vk::Buffer{vk::BufferType::Index, vk::BufferPurpose::NothingSpecial, *this->device, vk::hardware::MemoryResidency::CPUPersistent, dynamic_indices.size() * sizeof(unsigned int)};
                tz_report("IB Dynamic (%zu indices, %zu bytes total)", dynamic_indices.size(), dynamic_indices.size() * sizeof(unsigned int));

                std::byte* vtx_mem = static_cast<std::byte*>(this->dynamic_vertex_buffer.map_memory());
                for(const auto& vertex_region : vertex_regions)
                {
                    vertex_region.input.set_vertex_data(vtx_mem + vertex_region.offset);
                }

                unsigned int* idx_mem = static_cast<unsigned int*>(this->dynamic_index_buffer.map_memory());
                for(const auto& index_region : index_regions)
                {
                    index_region.input.set_index_data(idx_mem + index_region.offset);
                }
            }
        }

        for(std::size_t buf_res_id = 0; buf_res_id < this->buffer_components.size(); buf_res_id++ /*BufferComponentVulkan& buffer_component : this->buffer_components*/)
        {
            BufferComponentVulkan& buffer_component = this->buffer_components[buf_res_id];
            vk::BufferType buf_type;
            const tz::gl::ShaderMeta& meta = this->shader->get_meta();
            ShaderMetaValue value = meta.try_get_meta_value(buf_res_id).value_or(ShaderMetaValue::UBO);
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

            IResource* buffer_resource = buffer_component.get_resource();
            switch(buffer_resource->data_access())
            {
                case RendererInputDataAccess::StaticFixed:
                    buffer_component.set_buffer(vk::Buffer{buf_type, vk::BufferPurpose::TransferDestination, *this->device, vk::hardware::MemoryResidency::GPU, buffer_resource->get_resource_bytes().size_bytes()});
                break;
                case RendererInputDataAccess::DynamicFixed:
                    {
                        auto& dynamic_resource = static_cast<IDynamicResource&>(*buffer_resource);
                        buffer_component.set_buffer(vk::Buffer{buf_type, vk::BufferPurpose::NothingSpecial, *this->device, vk::hardware::MemoryResidency::CPUPersistent, dynamic_resource.get_resource_bytes().size_bytes()});
                        dynamic_resource.set_resource_data(static_cast<std::byte*>(buffer_component.get_buffer().map_memory()));
                    }
                break;
                default:
                    tz_error("Resource data access unsupported (Vulkan)");
                break;
            }
        }
    }

    const vk::Buffer& RendererBufferManagerVulkan::get_vertex_buffer() const
    {
        return this->vertex_buffer;
    }

    vk::Buffer& RendererBufferManagerVulkan::get_vertex_buffer()
    {
        return this->vertex_buffer;
    }

    const vk::Buffer& RendererBufferManagerVulkan::get_index_buffer() const
    {
        return this->index_buffer;
    }

    vk::Buffer& RendererBufferManagerVulkan::get_index_buffer()
    {
        return this->index_buffer;
    }

    const vk::Buffer& RendererBufferManagerVulkan::get_dynamic_vertex_buffer() const
    {
        return this->dynamic_vertex_buffer;
    }

    vk::Buffer& RendererBufferManagerVulkan::get_dynamic_vertex_buffer()
    {
        return this->dynamic_vertex_buffer;
    }

    const vk::Buffer& RendererBufferManagerVulkan::get_dynamic_index_buffer() const
    {
        return this->dynamic_index_buffer;
    }

    vk::Buffer& RendererBufferManagerVulkan::get_dynamic_index_buffer()
    {
        return this->dynamic_index_buffer;
    }

    std::span<const BufferComponentVulkan> RendererBufferManagerVulkan::get_buffer_components() const
    {
        return this->buffer_components;
    }

    std::span<BufferComponentVulkan> RendererBufferManagerVulkan::get_buffer_components()
    {
        return this->buffer_components;
    }

    RendererImageManagerVulkan::RendererImageManagerVulkan(RendererBuilderVulkan builder, RendererBuilderDeviceInfoVulkan device_info, const RenderPassVulkan& render_pass):
    device(device_info.device),
    physical_device(this->device->get_queue_family().dev),
    render_pass(&render_pass),
    swapchain(device_info.device_swapchain),
    maybe_swapchain_offscreen_imageview(std::nullopt),
    texture_components(),
    depth_image(std::nullopt),
    depth_imageview(std::nullopt),
    swapchain_framebuffers(),
    //requires_output_framebuffer(builder.get_output()->get_type() == RendererOutputType::Texture),
    output_texture_component(nullptr),
    texture_output_framebuffer()
    {
        if(vk::is_headless())
        {
            this->swapchain_framebuffers.reserve(1);
            this->maybe_swapchain_offscreen_imageview = vk::ImageView{*this->device, static_cast<const vk::Image&>(*this->swapchain)};
        }
        else
        {
            if(builder.get_output()->get_type() == RendererOutputType::Texture)
            {
                this->output_texture_component = static_cast<TextureOutput*>(builder.get_output())->get_first_colour_component();
            }
            else
            {
                const auto& as_swapchain = static_cast<const vk::Swapchain&>(*this->swapchain);
                this->swapchain_framebuffers.reserve(as_swapchain.get_image_views().size());
            }
            /*
            if(builder.get_output()->get_type() == RendererOutputType::Texture)
            {
                auto* texture_output = static_cast<const TextureOutput*>(builder.get_output());
                VkExtent2D output_component_dimensions;
                const TextureComponentVulkan* component = texture_output->get_first_colour_component();
                output_component_dimensions.width = component->get_image().get_width();
                output_component_dimensions.height = component->get_image().get_height();
                this->texture_output_framebuffer = vk::Framebuffer(this->render_pass->vk_get_render_pass(), component->get_view(), output_component_dimensions);
            }
            else
            {
                const auto& as_swapchain = static_cast<const vk::Swapchain&>(*this->swapchain);
                this->swapchain_framebuffers.reserve(as_swapchain.get_image_views().size());
            }
            */
            
        }
        
    }

    void RendererImageManagerVulkan::initialise_resources(std::vector<IResource*> renderer_texture_resources)
    {
        for(IResource* texture_resource : renderer_texture_resources)
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
                case TextureFormat::Bgra32UnsignedNorm:
                    format = vk::Image::Format::Bgra32UnsignedNorm;
                break;
                default:
                    format = vk::Image::Format::Undefined;
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

    void RendererImageManagerVulkan::setup_depth_image()
    {
        auto swapchain_width = static_cast<std::uint32_t>(this->swapchain->get_width());
        auto swapchain_height = static_cast<std::uint32_t>(this->swapchain->get_height());
        this->depth_image = vk::Image{*this->device, swapchain_width, swapchain_height, vk::Image::Format::DepthFloat32, {vk::Image::Usage::DepthStencilAttachment}, vk::hardware::MemoryResidency::GPU};
        this->depth_imageview = vk::ImageView{*this->device, this->depth_image.value()};
    }

    void RendererImageManagerVulkan::resize_output_component()
    {
        if(this->output_texture_component != nullptr)
        {
            this->output_texture_component->clear_and_resize(this->swapchain->get_width(), this->swapchain->get_height());
        }
    }

    void RendererImageManagerVulkan::setup_outout_framebuffer()
    {
        if(this->output_texture_component != nullptr)
        {
            //auto* texture_output = static_cast<const TextureOutput*>(builder.get_output());
            VkExtent2D output_component_dimensions;
            output_component_dimensions.width = this->output_texture_component->get_image().get_width();
            output_component_dimensions.height = this->output_texture_component->get_image().get_height();
            this->texture_output_framebuffer = vk::Framebuffer(this->render_pass->vk_get_render_pass(), this->output_texture_component->get_view(), output_component_dimensions);
        }
    }

    void RendererImageManagerVulkan::setup_swapchain_framebuffers()
    {
        if(this->output_texture_component != nullptr)
        {
            // If we're rendering into a texture, we don't need to setup swapchain framebuffers
            return;
        }
        this->swapchain_framebuffers.clear();
        using VkExtentComponentType = decltype(std::declval<VkExtent2D>().width);
        auto swapchain_width = static_cast<VkExtentComponentType>(this->swapchain->get_width());
        auto swapchain_height = static_cast<VkExtentComponentType>(this->swapchain->get_height());
        if(vk::is_headless())
        {
            VkExtent2D extent{.width = swapchain_width, .height = swapchain_height};
            if(this->depth_imageview.has_value())
            {
                this->swapchain_framebuffers.emplace_back(this->render_pass->vk_get_render_pass(), this->maybe_swapchain_offscreen_imageview.value(), this->depth_imageview.value(), extent);
            }
            else
            {
                this->swapchain_framebuffers.emplace_back(this->render_pass->vk_get_render_pass(), this->maybe_swapchain_offscreen_imageview.value(), extent);
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
                    this->swapchain_framebuffers.emplace_back(this->render_pass->vk_get_render_pass(), swapchain_view, this->depth_imageview.value(), VkExtent2D{.width = swapchain_width, .height = swapchain_height});
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

    std::span<const TextureComponentVulkan> RendererImageManagerVulkan::get_texture_components() const
    {
        return this->texture_components;
    }

    std::span<TextureComponentVulkan> RendererImageManagerVulkan::get_texture_components()
    {
        return this->texture_components;
    }

    const vk::Framebuffer* RendererImageManagerVulkan::get_texture_output() const
    {
        if(this->texture_output_framebuffer.has_value())
        {
            return &this->texture_output_framebuffer.value();
        }
        return nullptr;
    }

    RendererProcessorVulkan::RendererProcessorVulkan(RendererBuilderVulkan builder, RendererBuilderDeviceInfoVulkan device_info, std::vector<IRendererInput*> inputs, const RenderPassVulkan& render_pass):
    device(device_info.device),
    physical_device(this->device->get_queue_family().dev),
    render_pass(&render_pass),
    swapchain(device_info.device_swapchain),
    inputs(inputs),
    resource_descriptor_pool(std::nullopt),
    command_pool(*this->device, this->device->get_queue_family(), vk::CommandPool::RecycleBuffer),
    graphics_present_queue(this->device->get_hardware_queue()),
    draw_indirect_buffer{this->num_static_inputs() > 0 ? std::optional<vk::Buffer>{vk::Buffer{vk::BufferType::DrawIndirect, vk::BufferPurpose::TransferDestination, *this->device, vk::hardware::MemoryResidency::GPU, sizeof(DrawIndirectCommand) * this->num_static_inputs()}} : std::nullopt},
    draw_indirect_dynamic_buffer{this->num_dynamic_inputs() > 0 ? std::optional<vk::Buffer>{vk::Buffer{vk::BufferType::DrawIndirect, vk::BufferPurpose::TransferDestination, *this->device, vk::hardware::MemoryResidency::GPU, sizeof(DrawIndirectCommand) * this->num_dynamic_inputs()}} : std::nullopt},
    draw_cache(),
    requires_present(builder.get_output()->get_type() == RendererOutputType::Window),
    frame_admin(*this->device, vk::is_headless() ? 1 : RendererVulkan::frames_in_flight)
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
                vk::BufferType buf_type = buffer_manager.get_buffer_components()[i].get_buffer().get_type();
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
                        return;
                    break;
                }
                pool_builder.with_size(desc_type, image_count);
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
                    const vk::Buffer& resource_buffer = buffer_manager.get_buffer_components()[j].get_buffer();
                    request.add_buffer(resource_buffer, 0, VK_WHOLE_SIZE, j);
                }
                for(decltype(num_texture_resources) j = 0; j < num_texture_resources; j++)
                {
                    const TextureComponentVulkan& texture_component = image_manager.get_texture_components()[j];
                    request.add_image(texture_component.get_view(), texture_component.get_sampler(), j + num_buffer_resources);
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
        TZ_PROFZONE("Record Rendering Commands", TZ_PROFCOL_YELLOW);
        VkClearValue vk_clear_colour;
        vk_clear_colour.color = VkClearColorValue{.float32 = {clear_colour[0], clear_colour[1], clear_colour[2], clear_colour[3]}};
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
    }

    void RendererProcessorVulkan::clear_rendering_commands()
    {
        TZ_PROFZONE("Clear Rendering Commands", TZ_PROFCOL_YELLOW);
        for(std::size_t i = 0; i < this->get_view_count(); i++)
        {
            this->frame_admin.wait_for(i);
            this->command_pool[i].reset();
        }
    }

    void RendererProcessorVulkan::record_and_run_scratch_commands(RendererBufferManagerVulkan& buffer_manager, RendererImageManagerVulkan& image_manager)
    {
        TZ_PROFZONE("Record & Run Scratch Commands", TZ_PROFCOL_YELLOW);
        vk::Fence copy_fence{*this->device};
        copy_fence.signal();
        vk::CommandBuffer& scratch_buf = this->command_pool[this->get_view_count()];
        vk::Submit do_scratch_operation{vk::CommandBuffers{scratch_buf}, vk::SemaphoreRefs{}, vk::WaitStages{}, vk::SemaphoreRefs{}};

        // Run scratch commands to ensure inputs are sorted w.r.t vertex/index/draw-indirect buffers.
        {
            TZ_PROFZONE("Scratch : Inputs", TZ_PROFCOL_YELLOW);
            std::optional<vk::Buffer> draw_staging;
            if(this->num_static_inputs() > 0)
            {
                draw_staging = vk::Buffer{vk::BufferType::Staging, vk::BufferPurpose::TransferSource, *this->device, vk::hardware::MemoryResidency::CPU, sizeof(DrawIndirectCommand) * this->num_static_inputs()};
            }
            std::optional<vk::Buffer> draw_staging_dynamic;
            if(this->num_dynamic_inputs() > 0)
            {
                draw_staging_dynamic = vk::Buffer{vk::BufferType::Staging, vk::BufferPurpose::TransferSource, *this->device, vk::hardware::MemoryResidency::CPU, sizeof(DrawIndirectCommand) * this->num_dynamic_inputs()};
            }
            std::vector<std::byte> total_vertices, total_dynamic_vertices;
            std::vector<unsigned int> total_indices, total_dynamic_indices;
            DrawIndirectCommand* cmd = draw_staging.has_value() ? static_cast<DrawIndirectCommand*>(draw_staging->map_memory()) : nullptr;
            DrawIndirectCommand* dyn_cmd = draw_staging_dynamic.has_value() ? static_cast<DrawIndirectCommand*>(draw_staging_dynamic->map_memory()) : nullptr;
            std::uint32_t index_count_so_far = 0;
            std::uint32_t dyn_index_count_so_far = 0;
            std::uint32_t vertex_count_so_far = 0;
            std::uint32_t dyn_vertex_count_so_far = 0;

            {
                TZ_PROFZONE("Scratch : Fill indirect buffers", TZ_PROFCOL_RED);
                for(const IRendererInput* input : this->inputs)
                {
                    if(input != nullptr)
                    {
                        // Get vertices and indices.
                        std::span<const std::byte> input_vertices = input->get_vertex_bytes();
                        std::span<const unsigned int> input_indices = input->get_indices();
                        // Fill draw cmd
                        DrawIndirectCommand cur_cmd;
                        cur_cmd.indexCount = input->get_indices().size();
                        cur_cmd.instanceCount = 1;
                        cur_cmd.firstInstance = 0;

                        if(input->data_access() == RendererInputDataAccess::StaticFixed)
                        {
                            cur_cmd.firstIndex = index_count_so_far;
                            cur_cmd.vertexOffset = vertex_count_so_far;
                            std::copy(input_vertices.begin(), input_vertices.end(), std::back_inserter(total_vertices));
                            std::copy(input_indices.begin(), input_indices.end(), std::back_inserter(total_indices));
                            if(cmd != nullptr)
                            {
                                *cmd = cur_cmd;
                                cmd++;
                            }
                            index_count_so_far += input->index_count();
                            vertex_count_so_far += input->vertex_count();
                        }
                        else if(input->data_access() == RendererInputDataAccess::DynamicFixed)
                        {
                            cur_cmd.firstIndex = dyn_index_count_so_far;
                            cur_cmd.vertexOffset = dyn_vertex_count_so_far;
                            std::copy(input_vertices.begin(), input_vertices.end(), std::back_inserter(total_dynamic_vertices));
                            std::copy(input_indices.begin(), input_indices.end(), std::back_inserter(total_dynamic_indices));
                            if(dyn_cmd != nullptr)
                            {
                                *dyn_cmd = cur_cmd;
                                ++dyn_cmd;
                            }
                            dyn_index_count_so_far += input->index_count();
                            dyn_vertex_count_so_far += input->vertex_count();
                        }
                        
                    }
                }
            }
            if(draw_staging.has_value())
            {
                draw_staging->unmap_memory();
            }
            if(draw_staging_dynamic.has_value())
            {
                draw_staging_dynamic->unmap_memory();
            }

            // Setup input data transfers using the scratch buffers.
            if(index_count_so_far > 0)
            {
                // Part 1: Transfer vertex data.
                {
                    TZ_PROFZONE("Scratch : Transfer Vertex Data", TZ_PROFCOL_RED);
                    std::span<const std::byte> vertex_data = total_vertices;
                    copy_fence.signal();
                    vk::Buffer vertices_staging{vk::BufferType::Staging, vk::BufferPurpose::TransferSource, *this->device, vk::hardware::MemoryResidency::CPU, vertex_data.size_bytes()};
                    vertices_staging.write(vertex_data.data(), vertex_data.size_bytes());
                    {
                        vk::CommandBufferRecording transfer_vertices = scratch_buf.record();
                        transfer_vertices.buffer_copy_buffer(vertices_staging, buffer_manager.get_vertex_buffer(), vertex_data.size_bytes());
                    }

                    // Submit Part 1
                    do_scratch_operation(this->graphics_present_queue, copy_fence);
                    copy_fence.wait_for();
                }
                scratch_buf.reset();
                // Part 2: Transfer index data.
                {
                    TZ_PROFZONE("Scratch : Transfer Index Data", TZ_PROFCOL_RED);
                    std::span<const unsigned int> index_data = total_indices;
                    vk::Buffer indices_staging{vk::BufferType::Staging, vk::BufferPurpose::TransferSource, *this->device, vk::hardware::MemoryResidency::CPU, index_data.size_bytes()};
                    indices_staging.write(index_data.data(), index_data.size_bytes());
                    {
                        vk::CommandBufferRecording transfer_indices = scratch_buf.record();
                        transfer_indices.buffer_copy_buffer(indices_staging, buffer_manager.get_index_buffer(), index_data.size_bytes());
                    }
                    copy_fence.signal();
                    // Submit Part 2
                    do_scratch_operation(this->graphics_present_queue, copy_fence);
                    copy_fence.wait_for();
                }
                scratch_buf.reset();
            }

            // Part 3: Setup draw commands for the inputs.
            {
                TZ_PROFZONE("Scratch : Setup Initial Draw List", TZ_PROFCOL_RED);
                this->record_draw_list(this->all_inputs_once());
            }
        }

        {
            // Setup all components for buffer resources.
            TZ_PROFZONE("Scratch : Buffer Resources", TZ_PROFCOL_YELLOW);
            for(std::size_t i = 0; i < buffer_manager.get_buffer_components().size(); i++)
            {
                BufferComponentVulkan& buffer_component = buffer_manager.get_buffer_components()[i];
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
                    do_scratch_operation(this->graphics_present_queue, copy_fence);
                    copy_fence.wait_for();
                }
            }
        }

        {
            // Setup all components for texture resources.
            TZ_PROFZONE("Scratch : Texture Resources", TZ_PROFCOL_YELLOW);
            for(std::size_t i = 0; i < image_manager.get_texture_components().size(); i++)
            {
                TextureComponentVulkan& texture_component = image_manager.get_texture_components()[i];
                IResource* texture_resource = texture_component.get_resource();
                tz_report("Texture Resource (ResourceID: %zu, TextureComponentID: %zu, %zu bytes total)", buffer_manager.get_buffer_components().size() + i, i, texture_resource->get_resource_bytes().size_bytes());
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
                    do_scratch_operation(this->graphics_present_queue, copy_fence);
                    copy_fence.wait_for();
                }
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
            this->frame_admin.render_frame_headless(this->graphics_present_queue, this->command_pool, vk::WaitStages{vk::WaitStage::ColourAttachmentOutput});
        }
        else
        {
            if(this->requires_present)
            {
                this->frame_admin.render_frame(this->graphics_present_queue, static_cast<const vk::Swapchain&>(*this->swapchain), this->command_pool, vk::WaitStages{vk::WaitStage::ColourAttachmentOutput});
            }
            else
            {
                //this->frame_admin.render_frame(this->graphics_present_queue, static_cast<const vk::Swapchain&>(*this->swapchain), this->command_pool, vk::WaitStages{vk::WaitStage::ColourAttachmentOutput});

                this->frame_admin.submit_rendering(this->graphics_present_queue, this->command_pool, vk::WaitStages{vk::WaitStage::ColourAttachmentOutput});
            }
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

    std::size_t RendererProcessorVulkan::num_static_inputs() const
    {
        
        std::size_t total = std::accumulate(this->inputs.begin(), this->inputs.end(), 0, [](std::size_t accumulator, const IRendererInput* input)
        {
            return accumulator + (input->data_access() == RendererInputDataAccess::StaticFixed ? 1 : 0);
        });

        return total;
    }

    std::size_t RendererProcessorVulkan::num_dynamic_inputs() const
    {
        std::size_t total = std::accumulate(this->inputs.begin(), this->inputs.end(), 0, [](std::size_t accumulator, const IRendererInput* input)
        {
            return accumulator + (input->data_access() == RendererInputDataAccess::DynamicFixed ? 1 : 0);
        });
        return total;
    }

    std::size_t RendererProcessorVulkan::num_static_draws() const
    {
        std::size_t total = std::accumulate(this->draw_cache.begin(), this->draw_cache.end(), 0, [this](std::size_t accumulator, const RendererInputHandle& handle)
        {
            std::size_t handle_val = static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
            return accumulator + (this->inputs[handle_val]->data_access() == RendererInputDataAccess::StaticFixed ? 1 : 0);
        });
        return total;
    }
    
    std::size_t RendererProcessorVulkan::num_dynamic_draws() const
    {
        std::size_t total = std::accumulate(this->draw_cache.begin(), this->draw_cache.end(), 0, [this](std::size_t accumulator, const RendererInputHandle& handle)
        {
            std::size_t handle_val = static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
            return accumulator + (this->inputs[handle_val]->data_access() == RendererInputDataAccess::DynamicFixed ? 1 : 0);
        });
        return total;
    }

    void RendererProcessorVulkan::record_draw_list(const RendererDrawList& draws)
    {
        if(this->draws_match_cache(draws))
        {
            return;
        }
        // Firstly, retrieve an internal draw command for every input.
        std::unordered_map<const IRendererInput*, DrawIndirectCommand> input_draws;
        std::unordered_map<const IRendererInput*, DrawIndirectCommand> dynamic_input_draws;

        {
            std::size_t static_vtx_count = 0, static_idx_count = 0;
            std::size_t dynamic_vtx_count = 0, dynamic_idx_count = 0;
            for(const IRendererInput* input : this->inputs)
            {
                DrawIndirectCommand cmd;
                cmd.indexCount = input->index_count();
                cmd.instanceCount = 1;
                cmd.firstInstance = 0;
                switch(input->data_access())
                {
                    case RendererInputDataAccess::StaticFixed:
                        cmd.vertexOffset = static_vtx_count;
                        cmd.firstIndex = static_idx_count;
                        static_vtx_count += input->vertex_count();
                        static_idx_count += input->index_count();
                        input_draws[input] = cmd;
                    break;
                    case RendererInputDataAccess::DynamicFixed:
                        cmd.vertexOffset = dynamic_vtx_count;
                        cmd.firstIndex = dynamic_idx_count;
                        dynamic_vtx_count += input->vertex_count();
                        dynamic_idx_count += input->index_count();
                        dynamic_input_draws[input] = cmd;
                    break;
                    default:
                        tz_error("Unknown renderer input data access (Vulkan)");
                    break;
                }
            }
        }

        // Secondly, convert the draw list to a list of indirect draw commands.
        std::vector<DrawIndirectCommand> internal_draws;
        std::vector<DrawIndirectCommand> internal_draws_dynamic;
        internal_draws.reserve(draws.length());
        for(RendererInputHandle handle : draws)
        {
            std::size_t handle_val = static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
            const IRendererInput* input = this->inputs[handle_val];
            switch(input->data_access())
            {
                case RendererInputDataAccess::StaticFixed:
                    internal_draws.push_back(input_draws[input]);
                break;
                case RendererInputDataAccess::DynamicFixed:
                    internal_draws_dynamic.push_back(dynamic_input_draws[input]);
                break;
                default:
                        tz_error("Unknown renderer input data access (Vulkan)");
                break;
            }
        }
        tz_assert(internal_draws.size() + internal_draws_dynamic.size() == draws.length(), "Internal draw total didn't match number of inputs in draw list");

        vk::Fence copy_fence{*this->device};
        copy_fence.signal();
        vk::CommandBuffer& scratch_buf = this->command_pool[this->get_view_count()];
        vk::Submit do_scratch_operation{vk::CommandBuffers{scratch_buf}, vk::SemaphoreRefs{}, vk::WaitStages{}, vk::SemaphoreRefs{}};

        if(internal_draws.empty())
        {
            this->draw_indirect_buffer = std::nullopt;
        }
        else
        {
            std::size_t draw_size_bytes = internal_draws.size() * sizeof(DrawIndirectCommand);

            this->draw_indirect_buffer = vk::Buffer{vk::BufferType::DrawIndirect, vk::BufferPurpose::TransferDestination, *this->device, vk::hardware::MemoryResidency::GPU, draw_size_bytes};
            vk::Buffer draw_staging{vk::BufferType::Staging, vk::BufferPurpose::TransferSource, *this->device, vk::hardware::MemoryResidency::CPU, draw_size_bytes};
            draw_staging.write(internal_draws.data(), draw_size_bytes);
            {
                vk::CommandBufferRecording transfer_draw_commands = scratch_buf.record();
                transfer_draw_commands.buffer_copy_buffer(draw_staging, this->draw_indirect_buffer.value(), draw_size_bytes);
            }
            copy_fence.signal();
            do_scratch_operation(this->graphics_present_queue, copy_fence);
            copy_fence.wait_for();
        }

        if(internal_draws_dynamic.empty())
        {
            this->draw_indirect_dynamic_buffer = std::nullopt;
        }
        else
        {
            std::size_t draw_size_bytes = internal_draws_dynamic.size() * sizeof(DrawIndirectCommand);

            this->draw_indirect_dynamic_buffer = vk::Buffer{vk::BufferType::DrawIndirect, vk::BufferPurpose::TransferDestination, *this->device, vk::hardware::MemoryResidency::GPU, draw_size_bytes};
            vk::Buffer draw_staging{vk::BufferType::Staging, vk::BufferPurpose::TransferSource, *this->device, vk::hardware::MemoryResidency::CPU, draw_size_bytes};
            draw_staging.write(internal_draws_dynamic.data(), draw_size_bytes);
            {
                vk::CommandBufferRecording transfer_draw_commands = scratch_buf.record();
                transfer_draw_commands.buffer_copy_buffer(draw_staging, this->draw_indirect_dynamic_buffer.value(), draw_size_bytes);
            }
            copy_fence.signal();
            do_scratch_operation(this->graphics_present_queue, copy_fence);
            copy_fence.wait_for();
        }
        this->draw_cache = draws;
    }

    bool RendererProcessorVulkan::draws_match_cache(const RendererDrawList& draws) const
    {
        return this->draw_cache == draws;
    }

    RendererDrawList RendererProcessorVulkan::all_inputs_once() const
    {
        RendererDrawList list;
        for(std::size_t i = 0; i < this->inputs.size(); i++)
        {
            list.add(RendererInputHandle{static_cast<tz::HandleValue>(i)});
        }
        return list;
    }   

    RendererVulkan::RendererVulkan(RendererBuilderVulkan builder, RendererBuilderDeviceInfoVulkan device_info):
    renderer_inputs(this->copy_inputs(builder)),
    renderer_resources(),
    render_pass(this->make_simple_render_pass(builder, device_info)),
    buffer_manager(device_info, this->get_inputs(), builder.get_shader()),
    pipeline_manager(builder, device_info, this->render_pass),
    image_manager(builder, device_info, this->render_pass),
    processor(builder, device_info, this->get_inputs(), this->render_pass),
    clear_colour(),
    requires_depth_image(true /*TODO: Option to disable depth image*/)
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
        this->image_manager.setup_outout_framebuffer();
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
        tz_report("RendererVulkan (%zu input%s, %zu resource%s)", this->renderer_inputs.size(), this->renderer_inputs.size() == 1 ? "" : "s", this->renderer_resources.size(), this->renderer_resources.size() == 1 ? "" : "s");
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

    std::size_t RendererVulkan::input_count() const
    {
        return this->renderer_inputs.size();
    }

    std::size_t RendererVulkan::input_count_of(RendererInputDataAccess access) const
    {
        return std::accumulate(this->renderer_inputs.begin(), this->renderer_inputs.end(), 0, [access](std::size_t init, const std::unique_ptr<IRendererInput>& input_ptr)
        {
            return init + (input_ptr->data_access() == access ? 1 : 0);
        });
    }

    IRendererInput* RendererVulkan::get_input(RendererInputHandle handle)
    {
        std::size_t handle_val = static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
        if(this->renderer_inputs.size() <= handle_val)
        {
            return nullptr;
        }
        return this->renderer_inputs[handle_val].get();
    }

    std::size_t RendererVulkan::resource_count() const
    {
        return this->renderer_resources.size();
    }

    std::size_t RendererVulkan::resource_count_of(ResourceType type) const
    {
        return std::accumulate(this->renderer_resources.begin(), this->renderer_resources.end(), 0, [type](std::size_t init, const std::unique_ptr<IResource>& res_ptr)
        {
            return init + (res_ptr->get_type() == type ? 1 : 0);
        });
    }

    IResource* RendererVulkan::get_resource(ResourceHandle handle)
    {
        auto handle_value = static_cast<HandleValueUnderlying>(static_cast<HandleValue>(handle));
        if(this->renderer_resources.size() <= handle_value)
        {
            return nullptr;
        }
        return this->renderer_resources[handle_value].get();
    }

    IComponent* RendererVulkan::get_component(ResourceHandle handle)
    {
        IResource* resource = this->get_resource(handle);
        if(resource == nullptr)
        {
            return nullptr;
        }

        switch(resource->get_type())
        {
            case ResourceType::Buffer:
            {
                std::size_t buffer_id = this->resource_handle_to_buffer_id(handle);
                return &this->buffer_manager.get_buffer_components()[buffer_id];
            }
            break;
            case ResourceType::Texture:
            {
                std::size_t texture_id = this->resource_handle_to_texture_id(handle);
                return &this->image_manager.get_texture_components()[texture_id];
            }
            break;
            default:
                tz_error("Unknown ResourceType. Not yet implemented? (Vulkan)");
                return nullptr;
            break;
        }
    }

    void RendererVulkan::render()
    {
        this->processor.render();
    }

    void RendererVulkan::render(RendererDrawList draws)
    {
        TZ_PROFZONE("RendererVulkan::render(RendererDrawList)", TZ_PROFCOL_YELLOW);
        if(!this->processor.draws_match_cache(draws))
        {
            TZ_PROFZONE("Frontend VK : New Draw List", TZ_PROFCOL_YELLOW);
            this->processor.clear_rendering_commands();
            this->processor.record_draw_list(draws);
            this->processor.record_rendering_commands(this->pipeline_manager, this->buffer_manager, this->image_manager, this->clear_colour);
        }
        this->processor.render();
    }

    RenderPassVulkan RendererVulkan::make_simple_render_pass(const RendererBuilderVulkan& builder, const RendererBuilderDeviceInfoVulkan& device_info) const
    {
        RenderPassInfo info = detail::describe_renderer(builder, *device_info.creator_device);
        bool presentable_output = builder.get_output()->get_type() == RendererOutputType::Window;
        RenderPassBuilderVulkan pass_builder{info, presentable_output};
        return device_info.creator_device->vk_create_render_pass(pass_builder);
        /*
        RenderPassBuilderVulkan pass_builder;
        pass_builder.add_pass(builder.get_pass());
        if(builder.get_output()->get_type() != RendererOutputType::Window)
        {
            // We're not expecting to present the output to the window. Don't transition to presentable.
            pass_builder.set_presentable_output(false);
        }
        return device_info.creator_device->vk_create_render_pass(pass_builder);
        */
    }

    std::vector<std::unique_ptr<IRendererInput>> RendererVulkan::copy_inputs(const RendererBuilderVulkan builder)
    {
        std::vector<std::unique_ptr<IRendererInput>> input_duplicates;
        for(const IRendererInput* const input : builder.vk_get_inputs())
        {
            input_duplicates.push_back(input != nullptr ? input->unique_clone() : nullptr);
        }
        return input_duplicates;
    }

    std::vector<IRendererInput*> RendererVulkan::get_inputs()
    {
        std::vector<IRendererInput*> inputs;
        for(std::unique_ptr<IRendererInput>& input_ptr : this->renderer_inputs)
        {
            inputs.push_back(input_ptr.get());
        }
        return inputs;
    }

    void RendererVulkan::handle_resize()
    {
        this->pipeline_manager.reconstruct_pipeline();

        if(this->requires_depth_image)
        {
            this->image_manager.setup_depth_image();
        }

        this->image_manager.resize_output_component();
        this->image_manager.setup_outout_framebuffer();
        this->image_manager.setup_swapchain_framebuffers();

        std::vector<const IResource*> all_resources;
        for(const auto& resource_ptr : this->renderer_resources)
        {
            all_resources.push_back(resource_ptr.get());
        }
        this->processor.initialise_resource_descriptors(this->pipeline_manager, this->buffer_manager, this->image_manager, all_resources);
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