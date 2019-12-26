namespace tz::gl
{

    template<typename T>
    tz::mem::UniformPool<T> IBuffer::map_pool(MappingPurpose purpose)
    {
        tz::mem::Block mem_block = this->map(purpose);
        return {mem_block};
    }

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
    bool Buffer<T>::is_terminal() const
    {
        this->verify();
        this->verify_bound();
        GLint param;
        glGetBufferParameteriv(static_cast<GLenum>(T), GL_BUFFER_IMMUTABLE_STORAGE, &param);
        return (param == GL_FALSE) ? false : true;
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
    void Buffer<T>::terminal_resize(std::size_t size_bytes)
    {
        this->verify();
        this->verify_bound();
        this->verify_nonterminal();
        topaz_assert(!this->is_mapped(), "tz::gl::Buffer<T>::terminal_resize(", size_bytes, "): Cannot resize because this buffer is currently mapped.");
        glBufferStorage(static_cast<GLenum>(T), size_bytes, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    }

    template<BufferType T>
    void Buffer<T>::make_terminal()
    {
        this->verify();
        this->verify_bound();
        this->verify_nonterminal();
        topaz_assert(!this->is_mapped(), "tz::gl::Buffer<T>::make_terminal(): Cannot make terminal because the buffer is currently mapped.");
        // TODO: Maintain a copy of the underlying data first and copy that data back into the immutable data store.
        glBufferStorage(static_cast<GLenum>(T), this->size(), nullptr, GL_STATIC_DRAW);
    }

    template<BufferType T>
    tz::mem::Block Buffer<T>::map(MappingPurpose purpose)
    {
        this->verify();
        this->verify_bound();
        topaz_assert(!this->is_mapped(), "tz::gl::Buffer<T>::map(...): Attempted to map but we are already mapped");
        // We know for sure that we have a valid handle, it is currently bound and we're definitely not yet mapped.
        void* begin = nullptr;
        if(this->is_terminal())
            begin = glMapBufferRange(static_cast<GLenum>(T), 0, this->size(), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
        else
            begin = glMapBuffer(static_cast<GLenum>(T), static_cast<GLenum>(purpose));

        return {begin, this->size()};
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