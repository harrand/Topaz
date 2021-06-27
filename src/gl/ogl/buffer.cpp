#if TZ_OGL
#include "core/assert.hpp"
#include "gl/ogl/buffer.hpp"
#include <utility>
#include <cstring>

constexpr GLenum persistent_mapped_buffer_flags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

namespace tz::gl::ogl
{
    Buffer::Buffer(BufferType type, BufferPurpose purpose, BufferUsage usage, std::size_t bytes):
    buf(0),
    type(type),
    usage(usage),
    mapped_ptr(nullptr)
    {
        glCreateBuffers(1, &this->buf);
        GLenum flags = static_cast<GLenum>(purpose);
        switch(usage)
        {
            case BufferUsage::PersistentMapped:
                glNamedBufferStorage(this->buf, bytes, nullptr, GL_DYNAMIC_STORAGE_BIT | persistent_mapped_buffer_flags);
            break;
            default:
                glNamedBufferData(this->buf, bytes, nullptr, flags);
            break;
        }
    }

    Buffer::Buffer(Buffer&& move):
    buf(0),
    type(),
    usage(),
    mapped_ptr(nullptr)
    {
        *this = std::move(move);
    }

    Buffer::~Buffer()
    {
        glDeleteBuffers(1, &this->buf);
    }

    Buffer& Buffer::operator=(Buffer&& rhs)
    {
        std::swap(this->buf, rhs.buf);
        std::swap(this->type, rhs.type);
        std::swap(this->usage, rhs.usage);
        std::swap(this->mapped_ptr, rhs.mapped_ptr);
        return *this;
    }

    BufferType Buffer::get_type() const
    {
        return this->type;
    }

    void Buffer::write(const void* addr, std::size_t bytes)
    {
        if(this->mapped_ptr != nullptr)
        {
            std::memcpy(this->mapped_ptr, addr, bytes);
            return;
        }
        glNamedBufferSubData(this->buf, 0, static_cast<GLsizeiptr>(bytes), addr);
    }

    void* Buffer::map_memory()
    {
        if(this->mapped_ptr != nullptr)
        {
            return this->mapped_ptr;
        }
        GLint buf_size;
        glGetNamedBufferParameteriv(this->buf, GL_BUFFER_SIZE, &buf_size);
        GLenum flags;
        switch(this->usage)
        {
            case BufferUsage::PersistentMapped:
                flags = persistent_mapped_buffer_flags;
            break;
            default:
                flags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
            break;
        }
        this->mapped_ptr = glMapNamedBufferRange(this->buf, 0, buf_size, flags);
        return this->mapped_ptr;
    }

    void Buffer::unmap_memory()
    {
        glUnmapNamedBuffer(this->buf);
        this->mapped_ptr = nullptr;
    }

    void Buffer::bind() const
    {
        /*
        Vertex,
        Index,
        Uniform,
        DrawIndirect
        */
        GLenum buftype;
        switch(this->type)
        {
            case BufferType::Vertex:
                buftype = GL_ARRAY_BUFFER;
            break;
            case BufferType::Index:
                buftype = GL_ELEMENT_ARRAY_BUFFER;
            break;
            case BufferType::Uniform:
                buftype = GL_UNIFORM_BUFFER;
            break;
            case BufferType::DrawIndirect:
                buftype = GL_DRAW_INDIRECT_BUFFER;
            break;
            default:
                tz_error("Unrecognised BufferType (OpenGL)");
            break;
        }
        glBindBuffer(buftype, this->buf);
    }

    GLuint Buffer::native() const
    {
        return this->buf;
    }

    bool Buffer::is_null() const
    {
        return this->buf == 0;
    }

    Buffer Buffer::null()
    {
        return Buffer{};
    }

    Buffer::Buffer():
    buf(0),
    type(),
    usage(),
    mapped_ptr(nullptr)
    {

    }
}
#endif // TZ_OGL