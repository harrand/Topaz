namespace tz::gl
{

    template<BufferType T>
    void Buffer<T>::bind() const
    {
        IBuffer::verify();
        glBindBuffer(static_cast<GLenum>(T), this->handle);
    }

    template<BufferType T>
    void Buffer<T>::unbind() const
    {
        IBuffer::verify();
        glBindBuffer(static_cast<GLenum>(T), 0);
    }

    template<BufferType T>
    std::size_t Buffer<T>::size() const
    {
        if(T == BufferType::Array || T == BufferType::Index)
        {
            this->verify();
            this->verify_bound();
            GLint param;
            glGetBufferParameteriv(static_cast<GLenum>(T), GL_BUFFER_SIZE, &param);
            return static_cast<std::size_t>(param);
        }
        else
        {
            topaz_assert(false, "tz::gl::Buffer<T>::size(): Cannot query size of a buffer with this type. Can only accept VBOs or IBOs.");
            return 0;
        }
    }

    template<BufferType T>
    void Buffer<T>::resize(std::size_t size_bytes)
    {
        this->verify();
        this->verify_bound();
        topaz_assert(!this->is_mapped(), "tz::gl::Buffer<T>::resize(", size_bytes, "): Cannot resize because this buffer is currently mapped.");
        glBufferData(static_cast<GLenum>(T), size_bytes, nullptr, GL_STATIC_DRAW);
    }

    template<BufferType T>
    void* Buffer<T>::map(MappingPurpose purpose)
    {
        this->verify();
        this->verify_bound();
        topaz_assert(!this->is_mapped(), "tz::gl::Buffer<T>::map(...): Attempted to map but we are already mapped");
        // We know for sure that we have a valid handle, it is currently bound and we're definitely not yet mapped.
        return glMapBuffer(static_cast<GLenum>(T), static_cast<GLenum>(purpose));
    }

    template<BufferType T>
    void Buffer<T>::unmap()
    {
        this->verify();
        this->verify_bound();
        topaz_assert(this->is_mapped(), "tz::gl::Buffer<T>::unmap(): Attempted to unmap but we weren't already mapped");
        // We know for sure that we have a valid handle, it is currently bound and we're definitely mapped.
        glUnmapBuffer(static_cast<GLenum>(T));
    }

    template<BufferType T>
    bool Buffer<T>::is_mapped() const
    {
        this->verify();
        this->verify_bound();
        GLint param;
        glGetBufferParameteriv(static_cast<GLenum>(T), GL_BUFFER_MAPPED, &param);
        return param == GL_TRUE ? true : false;
    }
}