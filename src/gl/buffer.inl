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
        glBufferData(static_cast<GLenum>(T), size_bytes, nullptr, GL_STATIC_DRAW);
    }

    template<BufferType T>
    void* Buffer<T>::map(MappingPurpose purpose)
    {
        this->verify();
        this->verify_bound();
        return glMapBuffer(static_cast<GLenum>(T), static_cast<GLenum>(purpose));
    }

    template<BufferType T>
    void Buffer<T>::unmap()
    {
        this->verify();
        this->verify_bound();
        glUnmapBuffer(static_cast<GLenum>(T));
    }
}