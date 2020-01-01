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

    void IBuffer::retrieve_all(void* input_data) const
    {
        this->retrieve(0, this->size(), input_data);
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

    // Remember: SSBO == Buffer<BufferType::ShaderStorage>
    SSBO::Buffer(std::size_t layout_qualifier_id): IBuffer(), layout_qualifier_id(layout_qualifier_id){}

    void SSBO::bind() const
    {
        IBuffer::verify();
        constexpr GLenum type = static_cast<GLenum>(BufferType::ShaderStorage);
        glBindBuffer(type, this->handle);
        glBindBufferBase(type, this->layout_qualifier_id, this->handle);
    }

    void SSBO::unbind() const
    {
        IBuffer::verify();
        constexpr GLenum type = static_cast<GLenum>(BufferType::ShaderStorage);
        glBindBuffer(type, 0);
        glBindBufferBase(type, this->layout_qualifier_id, 0);
    }

    std::size_t SSBO::size() const
    {
        IBuffer::verify();
        this->verify_bound();
        GLint param;
        glGetBufferParameteriv(static_cast<GLenum>(BufferType::ShaderStorage), GL_BUFFER_SIZE, &param);
        return static_cast<std::size_t>(param);
    }

    bool SSBO::is_terminal() const
    {
        IBuffer::verify();
        this->verify_bound();
        GLint param;
        glGetBufferParameteriv(static_cast<GLenum>(BufferType::ShaderStorage), GL_BUFFER_IMMUTABLE_STORAGE, &param);
        return (param == GL_FALSE) ? false : true;
    }

    void SSBO::resize(std::size_t size_bytes)
    {
        IBuffer::verify();
        this->verify_bound();
        topaz_assert(!this->is_mapped(), "tz::gl::SSBO::resize(", size_bytes, "): Cannot resize because this buffer is currently mapped.");
        glBufferData(static_cast<GLenum>(BufferType::ShaderStorage), size_bytes, nullptr, GL_STATIC_DRAW);
    }

    void SSBO::retrieve(std::size_t offset, std::size_t size_bytes, void* input_data) const
    {
        IBuffer::verify();
        this->verify_bound();
        if(!this->is_terminal())
        {
            // Cannot do this while mapped if we're non-terminal.
            topaz_assert(!this->is_mapped(), "tz::gl::Buffer<T>::retrieve(", offset, ", ", size_bytes, ", ptr): Cannot retrieve because this buffer is both non-terminal and mapped. Cannot retrieve a non-terminal buffer if it is mapped.");
        }
        glGetBufferSubData(static_cast<GLenum>(BufferType::ShaderStorage), static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size_bytes), input_data);
    }

    void SSBO::send(std::size_t offset, tz::mem::Block output_block)
    {
        IBuffer::verify();
        this->verify_bound();
        if(!this->is_terminal())
            topaz_assert(!this->is_mapped(), "tz::gl::SSBO::send(", offset, ", tz::mem::Block (", output_block.size(), ")): Cannot send because this buffer is both non-terminal and mapped. Cannot send data to a non-terminal buffer if it is mapped.");
        topaz_assert(output_block.size() <= (this->size() - offset), "tz::gl::SSBO::send(", offset, ", tz::mem::Block (", output_block.size(), ")): Block of size ", output_block.size(), " cannot fit in the buffer of size ", this->size(), " at the offset ", offset);
        glBufferSubData(static_cast<GLenum>(BufferType::ShaderStorage), static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(output_block.size()), output_block.begin);
    }

    void SSBO::send(const void* output_data)
    {
        IBuffer::verify();
        this->verify_bound();
        if(!this->is_terminal())
            topaz_assert(!this->is_mapped(), "tz::gl::Buffer::send(void*): Cannot send because this buffer is both non-terminal and mapped. Cannot send data to a non-terminal buffer if it is mapped.");
        // This is alot less safe for obvious reasons. But it's very low overhead so we should support it.
        glBufferSubData(static_cast<GLenum>(BufferType::ShaderStorage), 0, static_cast<GLsizeiptr>(this->size()), output_data);
    }

    void SSBO::terminal_resize(std::size_t size_bytes)
    {
        IBuffer::verify();
        this->verify_bound();
        IBuffer::verify_nonterminal();
        topaz_assert(!this->is_mapped(), "tz::gl::SSBO<T>::terminal_resize(", size_bytes, "): Cannot resize because this buffer is currently mapped.");
        glBufferStorage(static_cast<GLenum>(BufferType::ShaderStorage), size_bytes, nullptr, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    }

    void SSBO::make_terminal()
    {
        IBuffer::verify();
        this->verify_bound();
        IBuffer::verify_nonterminal();
        topaz_assert(!this->is_mapped(), "tz::gl::SSBO<T>::make_terminal(): Cannot make terminal because the buffer is currently mapped.");
        // TODO: Maintain a copy of the underlying data first and copy that data back into the immutable data store.
        glBufferStorage(static_cast<GLenum>(BufferType::ShaderStorage), this->size(), nullptr, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    }

    std::size_t SSBO::get_binding_id() const
    {
        return this->layout_qualifier_id;
    }

    tz::mem::Block SSBO::map(MappingPurpose purpose)
    {
        IBuffer::verify();
        this->verify_bound();
        topaz_assert(!this->is_mapped(), "tz::gl::SSBO::map(...): Attempted to map but we are already mapped");
        // We know for sure that we have a valid handle, it is currently bound and we're definitely not yet mapped.
        void* begin = nullptr;
        if(this->is_terminal())
            begin = glMapBufferRange(static_cast<GLenum>(BufferType::ShaderStorage), 0, this->size(), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
        else
            begin = glMapBuffer(static_cast<GLenum>(BufferType::ShaderStorage), static_cast<GLenum>(purpose));

        return {begin, this->size()};
    }

    void SSBO::unmap()
    {
        IBuffer::verify();
        this->verify_bound();
        topaz_assert(this->is_mapped(), "tz::gl::Buffer<T>::unmap(): Attempted to unmap but we weren't already mapped");
        // We know for sure that we have a valid handle, it is currently bound and we're definitely mapped.
        glUnmapBuffer(static_cast<GLenum>(BufferType::ShaderStorage));
    }

    bool SSBO::is_mapped() const
    {
        IBuffer::verify();
        this->verify_bound();
        GLint param;
        glGetBufferParameteriv(static_cast<GLenum>(BufferType::ShaderStorage), GL_BUFFER_MAPPED, &param);
        return param == GL_TRUE ? true : false;
    }

    void SSBO::verify_bound() const
    {
        topaz_assert(this->operator==(tz::gl::bound::shader_storage_buffer()), "tz::gl::SSBO::verify_bound(): Verification Failed!");

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