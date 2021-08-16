#if TZ_OGL
#include "core/report.hpp"
#include "core/profiling/zone.hpp"
#include "core/tz.hpp"
#include "gl/impl/frontend/ogl/renderer.hpp"
#include "gl/resource.hpp"
#include "gl/output.hpp"
#include <numeric>

namespace tz::gl
{
    struct DrawIndirectCommand
    {
        unsigned int count;
        unsigned int instanceCount;
        unsigned int firstIndex;
        unsigned int baseVertex;
        unsigned int baseInstance;
    };

   RendererInputHandle RendererBuilderOGL::add_input(const IRendererInput& input)
   {
       auto sz = this->inputs.size();
       this->inputs.push_back(&input);
       return {static_cast<tz::HandleValue>(sz)};
   }

    const IRendererInput* RendererBuilderOGL::get_input(RendererInputHandle handle) const
    {
        std::size_t handle_val = static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
        tz_assert(handle_val < this->inputs.size(), "Handle value %zu does not belong to this Renderer. Does it perhaps belong to another?");
        return this->inputs[handle_val];
    }

    void RendererBuilderOGL::set_pass(RenderPassAttachment pass)
    {
        this->pass = pass;
    }
    
    RenderPassAttachment RendererBuilderOGL::get_pass() const
    {
        return this->pass;
    }

    void RendererBuilderOGL::set_output(IRendererOutput& output)
    {
        this->output = &output;
    }

    const IRendererOutput* RendererBuilderOGL::get_output() const
    {
        return this->output;
    }

    IRendererOutput* RendererBuilderOGL::get_output()
    {
        return this->output;
    }

    ResourceHandle RendererBuilderOGL::add_resource(const IResource& resource)
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
                tz_error("Unexpected resource type. Support for this resource type is not yet implemented (OGL)");
                return {static_cast<tz::HandleValue>(0)};
            break;
        }
    }

    const IResource* RendererBuilderOGL::get_resource(ResourceHandle handle) const
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


    void RendererBuilderOGL::set_culling_strategy(RendererCullingStrategy culling_strategy)
    {
        this->culling_strategy = culling_strategy;
    }

    RendererCullingStrategy RendererBuilderOGL::get_culling_strategy() const
    {
        return this->culling_strategy;
    }

    void RendererBuilderOGL::set_shader(const Shader& shader)
    {
        this->shader = &shader;
    }

    const Shader& RendererBuilderOGL::get_shader() const
    {
        return *this->shader;
    }

    std::span<const IResource* const> RendererBuilderOGL::ogl_get_buffer_resources() const
    {
        return {this->buffer_resources.begin(), this->buffer_resources.end()};
    }
    
    std::span<const IResource* const> RendererBuilderOGL::ogl_get_texture_resources() const
    {
        return {this->texture_resources.begin(), this->texture_resources.end()};
    }

    std::span<const IRendererInput* const> RendererBuilderOGL::ogl_get_inputs() const
    {
        return this->inputs;
    }


    RendererOGL::RendererOGL(RendererBuilderOGL builder, RendererDeviceInfoOGL device_info):
    vao(0),
    vbo(std::nullopt),
    ibo(std::nullopt),
    vbo_dynamic(std::nullopt),
    ibo_dynamic(std::nullopt),
    indirect_buffer(std::nullopt),
    indirect_buffer_dynamic(std::nullopt),
    resources(),
    resource_ubos(),
    resource_textures(),
    pass_attachment(builder.get_pass()),
    shader(&builder.get_shader()),
    inputs(this->copy_inputs(builder)),
    output(builder.get_output()),
    output_texture_component(nullptr),
    output_framebuffer(std::nullopt),
    draw_cache()
    {
        *device_info.on_resize = [this](){this->handle_resize();};
        if(this->output != nullptr)
        {
            if(this->output->get_type() == RendererOutputType::Texture)
            {
                this->output_texture_component = static_cast<TextureOutput*>(builder.get_output())->get_first_colour_component();

                this->setup_output_framebuffer();
            }
        }
        auto persistent_mapped_buffer_flags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

        if(this->pass_attachment != RenderPassAttachment::Colour)
        {
            glEnable(GL_DEPTH_TEST);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }
        switch(builder.get_culling_strategy())
        {
            case RendererCullingStrategy::NoCulling:
                glDisable(GL_CULL_FACE);
            break;
            case RendererCullingStrategy::CullFrontFaces:
                glEnable(GL_CULL_FACE);
                glCullFace(GL_FRONT);
            break;
            case RendererCullingStrategy::CullBackFaces:
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
            break;
            case RendererCullingStrategy::CullEverything:
                glEnable(GL_CULL_FACE);
                glCullFace(GL_FRONT_AND_BACK);
            break;
        }

        glCreateVertexArrays(1, &this->vao);
        glBindVertexArray(this->vao);

        struct DynamicInputMapRegion
        {
            IRendererDynamicInput& input;
            std::size_t offset;
            std::size_t length;
        };

        if(!builder.ogl_get_inputs().empty())
        {
            std::vector<std::byte> total_vertices;
            std::vector<unsigned int> total_indices;
            std::vector<std::byte> total_vertices_dynamic;
            std::vector<unsigned int> total_indices_dynamic;
            std::vector<DynamicInputMapRegion> vertex_regions, index_regions;

            bool any_static_geometry = false;
            bool any_dynamic_geometry = false;

            RendererElementFormat fmt;

            // Step 1: Compile all data.
            for(auto& input_ptr : this->inputs)
            {
                if(input_ptr == nullptr)
                {
                    continue;
                }
                IRendererInput& input = *input_ptr;
                fmt = input.get_format();
                this->format = input.get_format();
                std::span<const std::byte> input_vertices = input.get_vertex_bytes();
                std::span<const unsigned int> input_indices = input.get_indices();
                switch(input.data_access())
                {
                    case RendererInputDataAccess::StaticFixed:
                        std::copy(input_vertices.begin(), input_vertices.end(), std::back_inserter(total_vertices));
                        std::copy(input_indices.begin(), input_indices.end(), std::back_inserter(total_indices));
                        any_static_geometry = true;
                    break;
                    case RendererInputDataAccess::DynamicFixed:
                    {
                        std::size_t vertex_region_offset = total_vertices_dynamic.size();
                        std::size_t vertex_region_length = input_vertices.size();
                        std::copy(input_vertices.begin(), input_vertices.end(), std::back_inserter(total_vertices_dynamic));
                        std::size_t index_region_offset = total_indices_dynamic.size();
                        std::size_t index_region_length = input_indices.size();
                        std::copy(input_indices.begin(), input_indices.end(), std::back_inserter(total_indices_dynamic));
                        any_dynamic_geometry = true;

                        vertex_regions.push_back({.input = static_cast<IRendererDynamicInput&>(input), .offset = vertex_region_offset, .length = vertex_region_length});
                        index_regions.push_back({.input = static_cast<IRendererDynamicInput&>(input), .offset = index_region_offset, .length = index_region_length});
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
                this->vbo = ogl::Buffer{ogl::BufferType::Vertex, ogl::BufferPurpose::StaticDraw, ogl::BufferUsage::ReadWrite, total_vertices.size()};
                this->vbo->write(total_vertices.data(), total_vertices.size());
                tz_report("VB Static (%zu vertices, %zu bytes total)", total_vertices.size() / fmt.binding_size, total_vertices.size());
                std::size_t total_indices_size = total_indices.size() * sizeof(unsigned int);
                this->ibo = ogl::Buffer{ogl::BufferType::Index, ogl::BufferPurpose::StaticDraw, ogl::BufferUsage::ReadWrite, total_indices_size};
                this->ibo->write(total_indices.data(), total_indices_size);
                tz_report("IB Static (%zu indices, %zu bytes total)", total_indices.size(), total_indices_size);
            }
            if(any_dynamic_geometry)
            {
                this->vbo_dynamic = ogl::Buffer{ogl::BufferType::Vertex, ogl::BufferPurpose::DynamicDraw, ogl::BufferUsage::PersistentMapped, total_vertices_dynamic.size()};
                this->vbo_dynamic->write(total_vertices_dynamic.data(), total_vertices_dynamic.size());
                tz_report("VB Dynamic (%zu vertices, %zu bytes total)", total_vertices_dynamic.size() / fmt.binding_size, total_vertices_dynamic.size());
                std::size_t total_indices_dynamic_size = total_indices_dynamic.size() * sizeof(unsigned int);
                this->ibo_dynamic = ogl::Buffer{ogl::BufferType::Index, ogl::BufferPurpose::DynamicDraw, ogl::BufferUsage::PersistentMapped, total_indices_dynamic_size};
                this->ibo_dynamic->write(total_indices_dynamic.data(), total_indices_dynamic_size);
                tz_report("IB Dynamic (%zu indices, %zu bytes total)", total_indices_dynamic.size(), total_indices_dynamic_size);

                void* vtx_data = this->vbo_dynamic->map_memory();
                void* idx_data = this->ibo_dynamic->map_memory();

                std::byte* vtx_mem = static_cast<std::byte*>(vtx_data);
                for(const auto& vertex_region : vertex_regions)
                {
                    vertex_region.input.set_vertex_data(vtx_mem + vertex_region.offset);
                }

                unsigned int* idx_mem = static_cast<unsigned int*>(idx_data);
                for(const auto& index_region : index_regions)
                {
                    index_region.input.set_index_data(idx_mem + index_region.offset);
                }
            }

            // Step 3: Sort out formats (vertex array attributes)
            tz_assert(format.basis == RendererInputFrequency::PerVertexBasis, "Vertex data on a per-instance basis is not yet implemented");

            for(GLuint attrib_id = 0; attrib_id < static_cast<GLuint>(format.binding_attributes.length()); attrib_id++)
            {
                RendererAttributeFormat attrib_format = format.binding_attributes[attrib_id];
                GLint size;
                GLenum type;
                switch(attrib_format.type)
                {
                    case RendererComponentType::Float32:
                        size = 1;
                        type = GL_FLOAT;
                    break;
                    case RendererComponentType::Float32x2:
                        size = 2;
                        type = GL_FLOAT;
                    break;
                    case RendererComponentType::Float32x3:
                        size = 3;
                        type = GL_FLOAT;
                    break;
                    default:
                        tz_error("Support for this attribute format is not yet implemented");
                    break;
                }
                glEnableVertexArrayAttrib(this->vao, attrib_id);
                glVertexArrayAttribFormat(this->vao, attrib_id, size, type, GL_FALSE, static_cast<GLuint>(attrib_format.element_attribute_offset));
                glVertexArrayAttribBinding(this->vao, attrib_id, 0);
            }
        }

        std::vector<IResource*> buffer_resources;
        std::vector<IResource*> texture_resources;
        for(const IResource* buffer_resource : builder.ogl_get_buffer_resources())
        {
            this->resources.push_back(buffer_resource->unique_clone());
            buffer_resources.push_back(this->resources.back().get());
        }
        for(const IResource* texture_resource : builder.ogl_get_texture_resources())
        {
            this->resources.push_back(texture_resource->unique_clone());
            texture_resources.push_back(this->resources.back().get());
        }

        for(std::size_t i = 0 ; i < buffer_resources.size(); i++)
        {
            IResource* buffer_resource = buffer_resources[i];
            tz_report("Buffer Resource (ResourceID: %zu, BufferComponentID: %zu, %zu bytes total)", i, i, buffer_resource->get_resource_bytes().size_bytes());
            GLuint& buf = this->resource_ubos.emplace_back();
            glCreateBuffers(1, &buf);
            switch(buffer_resource->data_access())
            {
                case RendererInputDataAccess::StaticFixed:
                    glNamedBufferData(buf, buffer_resource->get_resource_bytes().size_bytes(), buffer_resource->get_resource_bytes().data(), GL_STATIC_DRAW);
                break;
                case RendererInputDataAccess::DynamicFixed:
                    {
                        auto& dynamic_buf = *static_cast<IDynamicResource*>(buffer_resource);
                        auto buf_bytes = dynamic_buf.get_resource_bytes().size_bytes();
                        glNamedBufferStorage(buf, buf_bytes, dynamic_buf.get_resource_bytes().data(), persistent_mapped_buffer_flags);
                        void* res_data = glMapNamedBufferRange(buf, 0, buf_bytes, persistent_mapped_buffer_flags);
                        dynamic_buf.set_resource_data(static_cast<std::byte*>(res_data));
                    }
                break;
                default:
                    tz_error("Resource type not yet implemented (OGL)");
                break;
            }
        }

        for(std::size_t i = 0; i < texture_resources.size(); i++)
        {
            auto* texture_resource = static_cast<TextureResource*>(texture_resources[i]);
            tz_report("Texture Resource (ResourceID: %zu, TextureComponentID: %zu, %zu bytes total)", buffer_resources.size() + i, i, texture_resource->get_resource_bytes().size_bytes());
            //GLuint& tex = this->resource_textures.emplace_back();
            //glCreateTextures(GL_TEXTURE_2D, 1, &tex);
            //GLenum internal_format, format, type;
            ogl::Texture::Format format;
            switch(texture_resource->get_format())
            {
                case TextureFormat::Rgba32Signed:
                    format = ogl::Texture::Format::Rgba32Signed;
                    //internal_format = GL_RGBA8;
                    //format = GL_RGBA;
                    //type = GL_BYTE;
                break;
                case TextureFormat::Rgba32Unsigned:
                    format = ogl::Texture::Format::Rgba32Unsigned;
                    //internal_format = GL_RGBA8;
                    //format = GL_RGBA;
                    //type = GL_UNSIGNED_BYTE;
                break;
                case TextureFormat::Rgba32sRGB:
                    format = ogl::Texture::Format::Rgba32sRGB;
                    //internal_format = GL_SRGB8_ALPHA8;
                    //format = GL_RGBA;
                    //type = GL_UNSIGNED_BYTE;
                break;
                case TextureFormat::DepthFloat32:
                    format = ogl::Texture::Format::DepthFloat32;
                    //internal_format = GL_DEPTH_COMPONENT32F;
                    //format = GL_DEPTH_COMPONENT;
                    //type = GL_FLOAT;
                break;
                case TextureFormat::Bgra32UnsignedNorm:
                    format = ogl::Texture::Format::Bgra32UnsignedNorm;
                break;
                default:
                    tz_error("Unrecogised Resource TextureFormat (OpenGL)");
                break;
            }

            TextureProperties gl_props = texture_resource->get_properties();
            ogl::TextureParameters ogl_params;
            auto to_ogl_filter = [](TexturePropertyFilter filter) -> GLint
            {
                switch(filter)
                {
                    case TexturePropertyFilter::Nearest:
                        return GL_NEAREST;
                    break;
                    case TexturePropertyFilter::Linear:
                        return GL_LINEAR;
                    break;
                    default:
                        tz_error("Unrecognised Resource TexturePropertyFilter (OpenGL)");
                        return 0;
                    break;
                }
            };

            auto to_ogl_addrmode = [](TextureAddressMode mode) -> GLint
            {
                switch(mode)
                {
                    case TextureAddressMode::ClampToEdge:
                        return GL_CLAMP_TO_EDGE;
                    break;
                    default:
                        tz_error("Unrecognised Resource TextureAddressMode (OpenGL)");
                        return 0;
                    break;
                }
            };

            ogl_params.min_filter = to_ogl_filter(gl_props.min_filter);
            ogl_params.mag_filter = to_ogl_filter(gl_props.mag_filter);
            ogl_params.tex_wrap_s = to_ogl_addrmode(gl_props.address_mode_u);
            ogl_params.tex_wrap_t = to_ogl_addrmode(gl_props.address_mode_v);
            ogl_params.tex_wrap_u = to_ogl_addrmode(gl_props.address_mode_w);
            /*
            glTextureParameteri(tex, GL_TEXTURE_WRAP_S, convert_address_mode(gl_props.address_mode_u));
			glTextureParameteri(tex, GL_TEXTURE_WRAP_T, convert_address_mode(gl_props.address_mode_v));
            glTextureParameteri(tex, GL_TEXTURE_WRAP_R, convert_address_mode(gl_props.address_mode_w));
			glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, convert_filter(gl_props.min_filter));
			glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, convert_filter(gl_props.min_filter));
            */

            GLsizei tex_w = texture_resource->get_width();
            GLsizei tex_h = texture_resource->get_height();
            ogl::Texture tex{tex_w, tex_h, format, ogl_params};
            tex.set_image_data(texture_resource->get_resource_bytes().data(), texture_resource->get_resource_bytes().size_bytes());
            this->resource_textures.emplace_back(texture_resource, std::move(tex));
            //glTextureStorage2D(tex, 1, internal_format, tex_w, tex_h);
            //glTextureSubImage2D(tex, 0, 0, 0, tex_w, tex_h, format, type, texture_resource->get_resource_bytes().data());
        }

        this->bind_draw_list(this->all_inputs_once());

        tz_report("RendererOGL (%zu input%s, %zu resource%s)", this->inputs.size(), this->inputs.size() == 1 ? "" : "s", this->resources.size(), this->resources.size() == 1 ? "" : "s");
    }

    RendererOGL::RendererOGL(RendererOGL&& move):
    vao(0),
    vbo(std::nullopt),
    ibo(std::nullopt),
    vbo_dynamic(std::nullopt),
    ibo_dynamic(std::nullopt),
    indirect_buffer(std::nullopt),
    indirect_buffer_dynamic(std::nullopt),
    resource_ubos(),
    pass_attachment(RenderPassAttachment::ColourDepth),
    shader(nullptr),
    output(nullptr)
    {
        *this = std::move(move);
    }

    RendererOGL::~RendererOGL()
    {
        glDeleteBuffers(static_cast<GLsizei>(this->resource_ubos.size()), this->resource_ubos.data());

        if(this->vao != 0)
        {
            glDeleteVertexArrays(1, &this->vao);
        }
    }

    RendererOGL& RendererOGL::operator=(RendererOGL&& rhs)
    {
        std::swap(this->vao, rhs.vao);
        std::swap(this->vbo, rhs.vbo);
        std::swap(this->ibo, rhs.ibo);
        std::swap(this->resource_ubos, rhs.resource_ubos);
        std::swap(this->resource_textures, rhs.resource_textures);
        std::swap(this->format, rhs.format);
        std::swap(this->pass_attachment, rhs.pass_attachment);
        std::swap(this->shader, rhs.shader);
        std::swap(this->inputs, rhs.inputs);
        std::swap(this->output, rhs.output);
        return *this;
    }

    void RendererOGL::set_clear_colour(tz::Vec4 clear_colour)
    {
        glClearColor(clear_colour[0], clear_colour[1], clear_colour[2], clear_colour[3]);
    }

    std::size_t RendererOGL::input_count() const
    {
        return this->inputs.size();
    }

    std::size_t RendererOGL::input_count_of(RendererInputDataAccess access) const
    {
        return std::accumulate(this->inputs.begin(), this->inputs.end(), 0, [access](std::size_t init, const std::unique_ptr<IRendererInput>& input_ptr)
        {
            if(input_ptr->data_access() == access)
            {
                return init + 1;
            }
            return init;
        });
    }

    IRendererInput* RendererOGL::get_input(RendererInputHandle handle)
    {
        auto handle_value = static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
        if(this->inputs.size() <= handle_value)
        {
            return nullptr;
        }
        return this->inputs[handle_value].get();
    }

    std::size_t RendererOGL::resource_count() const
    {
        return this->resources.size();
    }

    std::size_t RendererOGL::resource_count_of(ResourceType type) const
    {
        return std::accumulate(this->resources.begin(), this->resources.end(), 0, [type](std::size_t init, const std::unique_ptr<IResource>& res_ptr)
        {
            if(res_ptr->get_type() == type)
            {
                return init + 1;
            }
            return init;
        });
    }

    IResource* RendererOGL::get_resource(ResourceHandle handle)
    {
        auto handle_value = static_cast<HandleValueUnderlying>(static_cast<HandleValue>(handle));
        if(this->resources.size() <= handle_value)
        {
            return nullptr;
        }
        return this->resources[handle_value].get();
    }

    IComponent* RendererOGL::get_component([[maybe_unused]] ResourceHandle handle)
    {
        IResource* resource = this->get_resource(handle);
        if(resource == nullptr)
        {
            return nullptr;
        }
        switch(resource->get_type())
        {
            case ResourceType::Buffer:
                //std::size_t buffer_id = this->resource_handle_to_buffer_id(handle);
                tz_error("Retrieving Buffer Components are not yet supported (OpenGL)");
                return nullptr;
            break;
            case ResourceType::Texture:
            {
                std::size_t texture_id = this->resource_handle_to_texture_id(handle);
                return &this->resource_textures[texture_id];
            }
            break;
            default:
                tz_error("Unknown ResourceType. Not yet implemented? (OpenGL)");
                return nullptr;
            break;
        }
    }

    tz::Vec4 RendererOGL::get_clear_colour() const
    {
        GLfloat rgba[4];
        glGetFloatv(GL_COLOR_CLEAR_VALUE, rgba);
        return {static_cast<float>(rgba[0]), static_cast<float>(rgba[1]), static_cast<float>(rgba[2]), static_cast<float>(rgba[3])};
    }

    void RendererOGL::render()
    {
        TZ_PROFZONE("RendererOGL::render", TZ_PROFCOL_YELLOW);
        if(this->output == nullptr)
        {
            tz_report("[Warning]: RendererOGL::render() invoked with no output specified. The behaviour is undefined.");
        }
        else if(this->output->get_type() == RendererOutputType::Window)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, static_cast<GLsizei>(tz::window().get_width()), static_cast<GLsizei>(tz::window().get_height()));
        }
        else if(this->output->get_type() == RendererOutputType::Texture)
        {
            tz_assert(this->output_framebuffer.has_value(), "Renderer has a texture output, but no output framebuffer was ever created. Logic error (OpenGL)");
            this->output_framebuffer->bind();
            glViewport(0, 0, static_cast<GLsizei>(tz::window().get_width()), static_cast<GLsizei>(tz::window().get_height()));
        }

        GLenum buffer_bits;
        switch(this->pass_attachment)
        {
            case RenderPassAttachment::Colour:
            default:
                buffer_bits = GL_COLOR_BUFFER_BIT;
            break;
            case RenderPassAttachment::Depth:
                buffer_bits = GL_DEPTH_BUFFER_BIT;
            break;
            case RenderPassAttachment::ColourDepth:
                buffer_bits = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
            break;
        }
        glClear(buffer_bits);

        glBindVertexArray(this->vao);
        {
            TZ_PROFZONE("Frontend OGL : Bind Buffer Resources", TZ_PROFCOL_RED);
            for(std::size_t i = 0; i < this->resource_ubos.size(); i++)
            {
                GLuint res_ubo = this->resource_ubos[i];
                glBindBufferBase(GL_UNIFORM_BUFFER, static_cast<GLuint>(i), res_ubo);
            }
        }
        glUseProgram(this->shader->ogl_get_program_handle());
        {
            TZ_PROFZONE("Frontend OGL : Bind Texture Resources", TZ_PROFCOL_RED);
            for(std::size_t i = 0; i < this->resource_textures.size(); i++)
            {
                TextureComponentOGL& cur_tex = this->resource_textures[i];
                //GLuint res_tex = this->resource_textures[i];
                auto tex_location = static_cast<GLint>(this->resource_ubos.size() + i);

                //glBindTextureUnit(tex_location, res_tex);
                cur_tex.get_texture().bind_at(tex_location);
                glProgramUniform1i(this->shader->ogl_get_program_handle(), tex_location, tex_location);
            }
        }

        if(this->inputs.empty())
        {
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
        else
        {
            if(this->indirect_buffer.has_value())
            {
                TZ_PROFZONE("Frontend OGL : Static Inputs Draw", TZ_PROFCOL_RED);
                glVertexArrayVertexBuffer(this->vao, 0, this->vbo->native(), 0, static_cast<GLsizei>(this->format.binding_size));
                glVertexArrayElementBuffer(this->vao, this->ibo->native());
                this->indirect_buffer->bind();
                glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(this->num_static_draws()), sizeof(DrawIndirectCommand));
            }

            if(this->indirect_buffer_dynamic.has_value())
            {
                TZ_PROFZONE("Frontend OGL : Dynamic Inputs Draw", TZ_PROFCOL_RED);
                glVertexArrayVertexBuffer(this->vao, 0, this->vbo_dynamic->native(), 0, static_cast<GLsizei>(this->format.binding_size));
                glVertexArrayElementBuffer(this->vao, this->ibo_dynamic->native());
                this->indirect_buffer_dynamic->bind();
                glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(this->num_dynamic_draws()), sizeof(DrawIndirectCommand));
            }
        }
    }

    void RendererOGL::render(RendererDrawList draw_list)
    {
        if(!this->draws_match_cache(draw_list))
        {
            this->bind_draw_list(draw_list);
        }
        this->render();
    }

    void RendererOGL::setup_output_framebuffer()
    {
        if(this->output_texture_component == nullptr)
        {
            return;
        }
        const ogl::Texture& output_tex = this->output_texture_component->get_texture();
        GLsizei width = output_tex.get_width();
        GLsizei height = output_tex.get_height();

        this->output_framebuffer = ogl::Framebuffer{};
        this->output_depth_renderbuffer = ogl::Renderbuffer{width, height, ogl::Texture::Format::DepthFloat32};

        // For now we only support a single colour attachment and no depth attachments. That makes this really easy.
        this->output_framebuffer->attach(GL_COLOR_ATTACHMENT0, output_tex);
        this->output_framebuffer->attach(GL_DEPTH_ATTACHMENT, this->output_depth_renderbuffer.value());
        this->output_framebuffer->set_output(GL_COLOR_ATTACHMENT0);
    }

    void RendererOGL::bind_draw_list(const RendererDrawList& draws)
    {
        TZ_PROFZONE("RendererOGL::bind_draw_list", TZ_PROFCOL_YELLOW);
        if(this->draws_match_cache(draws))
        {
            return;
        }
        // Firstly, retrieve an internal draw command for every input.
        std::unordered_map<const IRendererInput*, DrawIndirectCommand> input_draws;
        std::unordered_map<const IRendererInput*, DrawIndirectCommand> dynamic_input_draws;

        {
            unsigned int static_vtx_count = 0, static_idx_count = 0;
            unsigned int dynamic_vtx_count = 0, dynamic_idx_count = 0;
            for(const auto& input_ptr : this->inputs)
            {
                const IRendererInput* input = input_ptr.get();
                DrawIndirectCommand cmd;
                cmd.count = static_cast<unsigned int>(input->index_count());
                cmd.instanceCount = 1;
                cmd.baseInstance = 0;
                switch(input->data_access())
                {
                    case RendererInputDataAccess::StaticFixed:
                        cmd.baseVertex = static_vtx_count;
                        cmd.firstIndex = static_idx_count;
                        static_vtx_count += input->vertex_count();
                        static_idx_count += input->index_count();
                        input_draws[input] = cmd;
                    break;
                    case RendererInputDataAccess::DynamicFixed:
                        cmd.baseVertex = dynamic_vtx_count;
                        cmd.firstIndex = dynamic_idx_count;
                        dynamic_vtx_count += input->vertex_count();
                        dynamic_idx_count += input->index_count();
                        dynamic_input_draws[input] = cmd;
                    break;
                    default:
                        tz_error("Unknown renderer input data access (OpenGL)");
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
            const IRendererInput* input = this->inputs[handle_val].get();
            switch(input->data_access())
            {
                case RendererInputDataAccess::StaticFixed:
                    internal_draws.push_back(input_draws[input]);
                break;
                case RendererInputDataAccess::DynamicFixed:
                    internal_draws_dynamic.push_back(dynamic_input_draws[input]);
                break;
                default:
                        tz_error("Unknown renderer input data access (OpenGL)");
                break;
            }
        }
        tz_assert(internal_draws.size() + internal_draws_dynamic.size() == draws.length(), "Internal draw total didn't match number of inputs in draw list");

        if(internal_draws.empty())
        {
            this->indirect_buffer = std::nullopt;
        }
        else
        {
            std::size_t draw_size_bytes = internal_draws.size() * sizeof(DrawIndirectCommand);
            if(!this->indirect_buffer.has_value())
            {
                this->indirect_buffer = ogl::Buffer{ogl::BufferType::DrawIndirect, ogl::BufferPurpose::StaticDraw, ogl::BufferUsage::ReadWrite, draw_size_bytes};
            }

            this->indirect_buffer->write(internal_draws.data(), draw_size_bytes);
        }

        if(internal_draws_dynamic.empty())
        {
            this->indirect_buffer_dynamic = std::nullopt;
        }
        else
        {
            std::size_t draw_size_bytes = internal_draws_dynamic.size() * sizeof(DrawIndirectCommand);
            if(!this->indirect_buffer_dynamic.has_value())
            {
                this->indirect_buffer_dynamic = ogl::Buffer{ogl::BufferType::DrawIndirect, ogl::BufferPurpose::StaticDraw, ogl::BufferUsage::ReadWrite, draw_size_bytes};
            }

            this->indirect_buffer_dynamic->write(internal_draws_dynamic.data(), draw_size_bytes);
        }
        this->draw_cache = draws;
    }

    bool RendererOGL::draws_match_cache(const RendererDrawList& list) const
    {
        return this->draw_cache == list;
    }

    RendererDrawList RendererOGL::all_inputs_once() const
    {
        RendererDrawList list;
        for(std::size_t i = 0; i < this->inputs.size(); i++)
        {
            list.add(RendererInputHandle{static_cast<tz::HandleValue>(i)});
        }
        return list;
    }

    std::vector<std::unique_ptr<IRendererInput>> RendererOGL::copy_inputs(const RendererBuilderOGL& builder)
    {
        std::vector<std::unique_ptr<IRendererInput>> inputs;
        for(const IRendererInput* const input : builder.ogl_get_inputs())
        {
            inputs.push_back(input->unique_clone());
        }
        return inputs;
    }

    std::vector<IRendererInput*> RendererOGL::get_inputs()
    {
        std::vector<IRendererInput*> inputs;
        for(const auto& input_ptr : this->inputs)
        {
            inputs.push_back(input_ptr.get());
        }
        return inputs;
    }

    std::size_t RendererOGL::num_static_inputs() const
    {
        std::size_t total = std::accumulate(this->inputs.begin(), this->inputs.end(), 0, [](std::size_t accumulator, const std::unique_ptr<IRendererInput>& input)
        {
            return accumulator + (input->data_access() == RendererInputDataAccess::StaticFixed ? 1 : 0);
        });

        return total;
    }

    std::size_t RendererOGL::num_dynamic_inputs() const
    {
        std::size_t total = std::accumulate(this->inputs.begin(), this->inputs.end(), 0, [](std::size_t accumulator, const std::unique_ptr<IRendererInput>& input)
        {
            return accumulator + (input->data_access() == RendererInputDataAccess::DynamicFixed ? 1 : 0);
        });

        return total;
    }

    std::size_t RendererOGL::num_static_draws() const
    {
        std::size_t total = std::accumulate(this->draw_cache.begin(), this->draw_cache.end(), 0, [this](std::size_t accumulator, const RendererInputHandle& handle)
        {
            std::size_t handle_val = static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
            return accumulator + (this->inputs[handle_val]->data_access() == RendererInputDataAccess::StaticFixed ? 1 : 0);
        });
        return total;
    }
    
    std::size_t RendererOGL::num_dynamic_draws() const
    {
        std::size_t total = std::accumulate(this->draw_cache.begin(), this->draw_cache.end(), 0, [this](std::size_t accumulator, const RendererInputHandle& handle)
        {
            std::size_t handle_val = static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
            return accumulator + (this->inputs[handle_val]->data_access() == RendererInputDataAccess::DynamicFixed ? 1 : 0);
        });
        return total;
    }

    void RendererOGL::resize_output_component()
    {
        if(this->output_texture_component != nullptr)
        {
            this->output_texture_component->clear_and_resize(tz::window().get_width(), tz::window().get_height());
        }
    }

    void RendererOGL::handle_resize()
    {
        this->resize_output_component();
        this->setup_output_framebuffer();
    }
}

#endif // TZ_OGL