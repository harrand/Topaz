#if TZ_OGL
#include "core/report.hpp"
#include "core/tz.hpp"
#include "gl/impl/ogl/renderer.hpp"

namespace tz::gl
{
   void RendererBuilderOGL::set_input(const IRendererInput& input)
   {
       this->input = &input;
       this->format = this->input->get_format();
   }

    const IRendererInput* RendererBuilderOGL::get_input() const
    {
        return this->input;
        //return this->format.value();
    }

    void RendererBuilderOGL::set_output(const IRendererOutput& output)
    {
        this->output = &output;
    }

    const IRendererOutput* RendererBuilderOGL::get_output() const
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

    void RendererBuilderOGL::set_culling_strategy(RendererCullingStrategy culling_strategy)
    {
        this->culling_strategy = culling_strategy;
    }

    RendererCullingStrategy RendererBuilderOGL::get_culling_strategy() const
    {
        return this->culling_strategy;
    }

    void RendererBuilderOGL::set_render_pass(const RenderPass& render_pass)
    {
        this->render_pass = &render_pass;
    }

    const RenderPass& RendererBuilderOGL::get_render_pass() const
    {
        tz_assert(this->render_pass != nullptr, "No render pass set");
        return *this->render_pass;
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

    RendererOGL::RendererOGL(RendererBuilderOGL builder):
    vao(0),
    vbo(0),
    ibo(0),
    resources(),
    resource_ubos(),
    resource_textures(),
    index_count(0),
    render_pass(&builder.get_render_pass()),
    shader(&builder.get_shader()),
    input(builder.get_input() == nullptr ? nullptr : builder.get_input()->unique_clone()),
    output(builder.get_output())
    {
        auto persistent_mapped_buffer_flags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

        if(builder.get_render_pass().requires_depth_image())
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

        glGenVertexArrays(1, &this->vao);
        glBindVertexArray(this->vao);
        if(builder.get_input() != nullptr)
        {
            // First fill buffers.
            {
                GLuint buffers[2];
                glCreateBuffers(2, buffers);
                this->vbo = buffers[0];
                this->ibo = buffers[1];
            }

            const IRendererInput& input = *this->input;

            auto vertices_size = input.get_vertex_bytes().size();
            auto vertices_size_bytes = input.get_vertex_bytes().size_bytes();
            auto indices_size = input.get_indices().size();
            auto indices_size_bytes = input.get_indices().size_bytes();
            tz_report("VBO (%zu vertices, %zu bytes total)", vertices_size, vertices_size_bytes);
            tz_report("IBO (%zu indices, %zu bytes total)", indices_size, indices_size_bytes);
            switch(input.data_access())
            {
                case RendererInputDataAccess::StaticFixed:
                {
                    glNamedBufferData(this->vbo, input.get_vertex_bytes().size_bytes(), input.get_vertex_bytes().data(), GL_STATIC_DRAW);
                    glNamedBufferData(this->ibo, input.get_indices().size_bytes(), input.get_indices().data(), GL_STATIC_DRAW);
                    this->index_count = static_cast<GLsizei>(input.get_indices().size());
                }
                break;
                case RendererInputDataAccess::DynamicFixed:
                    auto& dynamic_input = static_cast<IRendererDynamicInput&>(*this->input);
                    glNamedBufferStorage(this->vbo, dynamic_input.get_vertex_bytes().size_bytes(), dynamic_input.get_vertex_bytes().data(), persistent_mapped_buffer_flags);
                    glNamedBufferStorage(this->ibo, dynamic_input.get_indices().size_bytes(), dynamic_input.get_indices().data(), persistent_mapped_buffer_flags);
                    this->index_count = static_cast<GLsizei>(dynamic_input.get_indices().size());
                    void* vertex_data = glMapNamedBufferRange(this->vbo, 0, dynamic_input.get_vertex_bytes().size_bytes(), persistent_mapped_buffer_flags);
                    void* index_data = glMapNamedBufferRange(this->ibo, 0, dynamic_input.get_indices().size_bytes(), persistent_mapped_buffer_flags);
                    dynamic_input.set_vertex_data(static_cast<std::byte*>(vertex_data));
                    dynamic_input.set_index_data(static_cast<unsigned int*>(index_data));
                break;
            }
            
            glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
            // Then sort out formats (vertex array attributes)
            RendererElementFormat format = builder.get_input()->get_format();
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

                auto to_ptr = [](std::size_t offset)->const void*{return reinterpret_cast<const void*>(offset);};
                glEnableVertexAttribArray(attrib_id);
                glVertexAttribPointer(attrib_id, size, type, GL_FALSE, static_cast<GLsizei>(format.binding_size), to_ptr(attrib_format.element_attribute_offset));
            }
            glBindBuffer(GL_ARRAY_BUFFER, 0);
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
            GLuint& tex = this->resource_textures.emplace_back();
            glCreateTextures(GL_TEXTURE_2D, 1, &tex);
            GLenum internal_format, format, type;
            switch(texture_resource->get_format())
            {
                case TextureFormat::Rgba32Signed:
                    internal_format = GL_RGBA8;
                    format = GL_RGBA;
                    type = GL_BYTE;
                break;
                case TextureFormat::Rgba32Unsigned:
                    internal_format = GL_RGBA8;
                    format = GL_RGBA;
                    type = GL_UNSIGNED_BYTE;
                break;
                case TextureFormat::Rgba32sRGB:
                    internal_format = GL_SRGB8_ALPHA8;
                    format = GL_RGBA;
                    type = GL_UNSIGNED_BYTE;
                break;
                case TextureFormat::DepthFloat32:
                    internal_format = GL_DEPTH_COMPONENT32F;
                    format = GL_DEPTH_COMPONENT;
                    type = GL_FLOAT;
                break;
            }
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, internal_format, texture_resource->get_width(), texture_resource->get_height(), 0, format, type, texture_resource->get_resource_bytes().data());
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        tz_report("RendererOGL (%s, %zu resource%s)", this->input != nullptr ? "Input" : "No Input", this->resources.size(), this->resources.size() == 1 ? "" : "s");
    }

    RendererOGL::RendererOGL(RendererOGL&& move):
    vao(0),
    vbo(0),
    ibo(0),
    resource_ubos(),
    index_count(0),
    render_pass(nullptr),
    shader(nullptr),
    output(nullptr)
    {
        *this = std::move(move);
    }

    RendererOGL::~RendererOGL()
    {
        if(this->vbo != 0)
        {
            glDeleteBuffers(1, &this->vbo);
        }

        if(this->ibo != 0)
        {
            glDeleteBuffers(1, &this->ibo);
        }

        glDeleteBuffers(static_cast<GLsizei>(this->resource_ubos.size()), this->resource_ubos.data());
        glDeleteTextures(static_cast<GLsizei>(this->resource_textures.size()), this->resource_textures.data());

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
        std::swap(this->index_count, rhs.index_count);
        std::swap(this->render_pass, rhs.render_pass);
        std::swap(this->shader, rhs.shader);
        std::swap(this->output, rhs.output);
        return *this;
    }

    void RendererOGL::set_clear_colour(tz::Vec4 clear_colour)
    {
        glClearColor(clear_colour[0], clear_colour[1], clear_colour[2], clear_colour[3]);
    }

    IRendererInput* RendererOGL::get_input()
    {
        return this->input.get();
    }

    IResource* RendererOGL::get_resource(ResourceHandle handle)
    {
        auto handle_value = static_cast<HandleValueUnderlying>(static_cast<HandleValue>(handle));
        return this->resources[handle_value].get();
    }

    tz::Vec4 RendererOGL::get_clear_colour() const
    {
        GLfloat rgba[4];
        glGetFloatv(GL_COLOR_CLEAR_VALUE, rgba);
        return {rgba[0], rgba[1], rgba[2], rgba[3]};
    }

    void RendererOGL::render()
    {
        if(this->output == nullptr)
        {
            tz_report("[Warning]: RendererOGL::render() invoked with no output specified. The behaviour is undefined.");
        }
        else if(this->output->get_type() == RendererOutputType::Window)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, static_cast<GLsizei>(tz::window().get_width()), static_cast<GLsizei>(tz::window().get_height()));
        }

        auto attachment = this->render_pass->ogl_get_attachments()[0];
        GLenum buffer_bits;
        switch(attachment)
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
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibo);
        for(std::size_t i = 0; i < this->resource_ubos.size(); i++)
        {
            GLuint res_ubo = this->resource_ubos[i];
            glBindBufferBase(GL_UNIFORM_BUFFER, static_cast<GLuint>(i), res_ubo);
        }
        glUseProgram(this->shader->ogl_get_program_handle());
        
        for(std::size_t i = 0; i < this->resource_textures.size(); i++)
        {
            GLuint res_tex = this->resource_textures[i];
            auto tex_location = static_cast<GLint>(this->resource_ubos.size() + i);
            glActiveTexture(GL_TEXTURE0 + tex_location);
            glBindTexture(GL_TEXTURE_2D, res_tex);
            glUniform1i(tex_location, res_tex);
        }

        glDrawElements(GL_TRIANGLES, this->index_count, GL_UNSIGNED_INT, nullptr);
    }
}

#endif // TZ_OGL