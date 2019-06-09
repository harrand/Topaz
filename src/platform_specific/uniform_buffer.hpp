//
// Created by Harrand on 09/06/2019.
//

#ifndef TOPAZ_UNIFORM_BUFFER_HPP
#define TOPAZ_UNIFORM_BUFFER_HPP
#include "platform_specific/generic_buffer.hpp"
#ifdef TOPAZ_OPENGL

namespace tz::platform
{
    class OGLUniformBuffer : public OGLGenericBuffer<OGLBufferType::UNIFORM_STORAGE>
    {
    public:
        OGLUniformBuffer(GLuint binding_id);
        virtual void bind() const override;
        friend class OGLShaderProgram;
    private:
        GLuint binding_id;
    };
}
#endif
#endif //TOPAZ_UNIFORM_BUFFER_HPP
