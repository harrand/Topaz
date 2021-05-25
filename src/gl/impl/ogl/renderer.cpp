#if TZ_OGL
#include "core/report.hpp"
#include "gl/impl/ogl/renderer.hpp"

namespace tz::gl
{
    /*
    void RendererBuilderOGL::set_input_format(RendererElementFormat element_format)
    {
        this->format = {element_format};
    }
    */
   void RendererBuilderOGL::set_input(const IRendererInput& input)
   {
       this->input = &input;
       this->format = this->input->get_format();
   }

    const IRendererInput* RendererBuilderOGL::get_input() const
    {
        tz_assert(this->format.has_value(), "RendererBuilder has not had element format set yet");
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

    RendererOGL::RendererOGL(RendererBuilderOGL builder):
    vao(0),
    vbo(0),
    ibo(0),
    index_count(0),
    render_pass(&builder.get_render_pass()),
    shader(&builder.get_shader()),
    input(builder.get_input()->unique_clone()),
    output(builder.get_output())
    {
        switch(builder.get_render_pass().ogl_get_attachments()[0])
        {
            case RenderPassAttachment::ColourDepth:
            case RenderPassAttachment::Depth:
                glEnable(GL_DEPTH_TEST);
            break;
            case RenderPassAttachment::Colour:
                glDisable(GL_DEPTH_TEST);
            break;
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
                    this->index_count = input.get_indices().size();
                }
                break;
                case RendererInputDataAccess::DynamicFixed:
                    auto& dynamic_input = static_cast<IRendererDynamicInput&>(*this->input);
                    auto persistent_mapped_buffer_flags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
                    glNamedBufferStorage(this->vbo, dynamic_input.get_vertex_bytes().size_bytes(), dynamic_input.get_vertex_bytes().data(), persistent_mapped_buffer_flags);
                    glNamedBufferStorage(this->ibo, dynamic_input.get_indices().size_bytes(), dynamic_input.get_indices().data(), persistent_mapped_buffer_flags);
                    this->index_count = dynamic_input.get_indices().size();
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

            for(std::size_t attrib_id = 0; attrib_id < format.binding_attributes.length(); attrib_id++)
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
                glVertexAttribPointer(attrib_id, size, type, GL_FALSE, format.binding_size, to_ptr(attrib_format.element_attribute_offset));
            }
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        tz_report("RendererOGL (Input = %p)", builder.get_input());
    }

    RendererOGL::RendererOGL(RendererOGL&& move):
    vao(0),
    vbo(0),
    ibo(0),
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

    tz::Vec4 RendererOGL::get_clear_colour() const
    {
        GLfloat rgba[4];
        glGetFloatv(GL_COLOR_CLEAR_VALUE, rgba);
        return {rgba[0], rgba[1], rgba[2], rgba[3]};
    }

    void RendererOGL::render()
    {
        if(this->output != nullptr)
        {
            this->output->set_render_target();
        }
        else
        {
            tz_report("[Warning]: RendererOGL::render() invoked with no output specified. The behaviour is undefined.");
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
        glUseProgram(this->shader->ogl_get_program_handle());
        glDrawElements(GL_TRIANGLES, this->index_count, GL_UNSIGNED_INT, nullptr);
    }
}


#endif // TZ_OGL