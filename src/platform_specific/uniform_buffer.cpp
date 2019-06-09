//
// Created by Harrand on 09/06/2019.
//

#include "uniform_buffer.hpp"

#ifdef TOPAZ_OPENGL
namespace tz::platform
{
    OGLUniformBuffer::OGLUniformBuffer(GLuint binding_id): OGLGenericBuffer<OGLBufferType::UNIFORM_STORAGE>(), binding_id(binding_id){}

    void OGLUniformBuffer::bind() const
    {
        OGLGenericBuffer<OGLBufferType::UNIFORM_STORAGE>::bind();
        glBindBufferBase(GL_UNIFORM_BUFFER, this->binding_id, this->buffer_handle);
    }
}
#endif