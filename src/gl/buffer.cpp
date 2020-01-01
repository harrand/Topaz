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

    bool IBuffer::empty() const
    {
        return this->size() == 0;
    }

    bool IBuffer::valid() const
    {
        this->verify();
        return glIsBuffer(this->handle);
    }
    
    bool IBuffer::operator==(BufferHandle handle) const
    {
        return this->handle == handle;
    }

    bool IBuffer::operator!=(BufferHandle handle) const
    {
        return this->handle != handle;
    }

    void IBuffer::verify() const
    {
        #if TOPAZ_DEBUG
            topaz_assert(this->handle != 0, "IBuffer::verify(): Handle was invalid (", this->handle, "). Perhaps this was moved from?");
        #endif
    }

    void IBuffer::verify_nonterminal() const
    {
        #if TOPAZ_DEBUG
            topaz_assert(!this->is_terminal(), "IBuffer::verify_nonterminal(): Buffer is terminal!");
        #endif
    }

    namespace bound
    {
        int vertex_buffer()
        {
            int handle;
            glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &handle);
            return handle;
        }

        int index_buffer()
        {
            int handle;
            glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &handle);
            return handle;
        }

        int shader_storage_buffer()
        {
            int handle;
            glGetIntegerv(GL_SHADER_STORAGE_BUFFER_BINDING, &handle);
            return handle;
        }

        int buffer(BufferType type)
        {
            switch(type)
            {
                case BufferType::Array:
                    return vertex_buffer();
                break;
                case BufferType::Index:
                    return index_buffer();
                break;
                case BufferType::ShaderStorage:
                    return shader_storage_buffer();
                break;
                default:
                    topaz_assert("tz::gl::bound::buffer(", static_cast<GLenum>(type), "): Checking the current bound buffer of this type is not yet implemented, or unsupported.");
                    return 0;
                break;
            }
        }
    }
}