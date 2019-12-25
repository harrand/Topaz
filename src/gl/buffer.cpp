//
// Created by Harrand on 25/12/2019.
//

#include "gl/buffer.hpp"
#include "core/debug/assert.hpp"

namespace tz::gl
{
    IBuffer::IBuffer(): handle(0)
    {
        glGenBuffers(1, &this->handle);
    }

    IBuffer::~IBuffer()
    {
        glDeleteBuffers(1, &this->handle);
    }

    bool IBuffer::operator==(BufferHandle handle) const
    {
        return this->handle == handle;
    }

    void IBuffer::verify() const
    {
        #if TOPAZ_DEBUG
            topaz_assert(this->handle != 0, "IBuffer::verify(): Handle was invalid (", this->handle, "). Perhaps this was moved from?");
        #endif
    }

    namespace bound
    {
        int vertex_buffer()
        {
            int handle;
            glGetIntegerv(GL_VERTEX_ARRAY_BUFFER_BINDING, &handle);
            return handle;
        }
    }
}