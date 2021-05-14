#if TZ_OGL
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

    RendererElementFormat RendererBuilderOGL::get_input_format() const
    {
        tz_assert(this->format.has_value(), "RendererBuilder has not had element format set yet");
        return this->format.value();
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
        // TODO: Do we need to do something with this later?
    }

    const RenderPass& RendererBuilderOGL::get_render_pass() const
    {
        tz_error("Renderer render pass integration is not yet implemented for OpenGL");
        return *static_cast<const RenderPass*>(nullptr);
    }

    void RendererBuilderOGL::set_shader(const Shader& shader)
    {
        // TODO: Do we need to do something with this later? Almost certainly.
    }

    const Shader& RendererBuilderOGL::get_shader() const
    {
        tz_error("Renderer shader integration is not yet implemented for OpenGL");
        return *static_cast<const Shader*>(nullptr);
    }

    RendererOGL::RendererOGL(RendererBuilderOGL builder):
    vao(0)
    {
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

        RendererElementFormat format = builder.get_input_format();
        tz_assert(format.basis == RendererInputFrequency::PerVertexBasis, "Vertex data on a per-instance basis is not yet implemented");
        glGenVertexArrays(1, &this->vao);

        for(std::size_t attrib_id = 0; attrib_id < format.binding_attributes.length(); attrib_id++)
        {
            RendererAttributeFormat attrib_format = format.binding_attributes[attrib_id];
            GLint size;
            GLenum type;
            switch(attrib_format.type)
            {
                case RendererComponentType::Float32:
                    size = sizeof(float);
                    type = GL_FLOAT;
                break;
                case RendererComponentType::Float32x2:
                    size = sizeof(float) * 2;
                    type = GL_FLOAT;
                break;
                case RendererComponentType::Float32x3:
                    size = sizeof(float) * 3;
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
    }

    RendererOGL::RendererOGL(RendererOGL&& move):
    vao(0)
    {
        *this = std::move(move);
    }

    RendererOGL::~RendererOGL()
    {
        if(this->vao != 0)
        {
            glDeleteVertexArrays(1, &this->vao);
        }
    }

    RendererOGL& RendererOGL::operator=(RendererOGL&& rhs)
    {
        std::swap(this->vao, rhs.vao);
        return *this;
    }
}


#endif // TZ_OGL