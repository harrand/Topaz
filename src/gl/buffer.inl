namespace tz::gl
{

    template<typename Iter>
    void IBuffer::send_range(Iter begin, Iter end)
    {
        const std::size_t size_elements = std::distance(begin, end);
        constexpr std::size_t element_size = sizeof(typename Iter::value_type);
        const std::size_t size_bytes = size_elements * element_size;
        topaz_assert(this->size() >= size_bytes, "tz::gl::IBuffer::send_range<Iter>(...): Range had distance of ", size_elements, " elements, equating to ", size_bytes, " bytes. This is too large for this buffer of size", this->size(), " bytes...");
        // Definitely is enough space to hold every element.
        // We can't assume that dereferencing all iterators in the range yields a contiguous set of data. We will make one!
        std::vector<typename Iter::value_type> contiguous_values;
        for(Iter i = begin; i != end; i++)
        {
            // Make a copy of all of the data.
            contiguous_values.push_back(*i);
        }
        topaz_assert(contiguous_values.size() == size_elements, "tz::gl::IBuffer::send_range<Iter>(...): Temporary contiguous buffer of the copied range had unexpected size. Expected size ", size_elements, " but got ", contiguous_values.size());
        // Now we have all the data we need, we can just send it now.
        this->send(0, tz::mem::Block{contiguous_values.data(), size_bytes});
    }

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
        IBuffer::verify();
        this->verify_bound();
        GLint param;
        glGetBufferParameteriv(static_cast<GLenum>(T), GL_BUFFER_SIZE, &param);
        return static_cast<std::size_t>(param);
    }

    template<BufferType T>
    bool Buffer<T>::is_terminal() const
    {
        IBuffer::verify();
        this->verify_bound();
        GLint param;
        glGetBufferParameteriv(static_cast<GLenum>(T), GL_BUFFER_IMMUTABLE_STORAGE, &param);
        return (param == GL_FALSE) ? false : true;
    }

    template<BufferType T>
    void Buffer<T>::resize(std::size_t size_bytes)
    {
        IBuffer::verify();
        this->verify_bound();
        topaz_assert(!this->is_mapped(), "tz::gl::Buffer<T>::resize(", size_bytes, "): Cannot resize because this buffer is currently mapped.");
        glBufferData(static_cast<GLenum>(T), size_bytes, nullptr, GL_STATIC_DRAW);
    }

    template<BufferType T>
    void Buffer<T>::retrieve(std::size_t offset, std::size_t size_bytes, void* input_data) const
    {
        IBuffer::verify();
        this->verify_bound();
        if(!this->is_terminal())
        {
            // Cannot do this while mapped if we're non-terminal.
            topaz_assert(!this->is_mapped(), "tz::gl::Buffer<T>::retrieve(", offset, ", ", size_bytes, ", ptr): Cannot retrieve because this buffer is both non-terminal and mapped. Cannot retrieve a non-terminal buffer if it is mapped.");
        }
        glGetBufferSubData(static_cast<GLenum>(T), static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size_bytes), input_data);
    }

    template<BufferType T>
    void Buffer<T>::send(std::size_t offset, tz::mem::Block output_block)
    {
        IBuffer::verify();
        this->verify_bound();
        if(!this->is_terminal())
            topaz_assert(!this->is_mapped(), "tz::gl::Buffer::send(", offset, ", tz::mem::Block (", output_block.size(), ")): Cannot send because this buffer is both non-terminal and mapped. Cannot send data to a non-terminal buffer if it is mapped.");
        topaz_assert(output_block.size() <= (this->size() - offset), "tz::gl::Buffer::send(", offset, ", tz::mem::Block (", output_block.size(), ")): Block of size ", output_block.size(), " cannot fit in the buffer of size ", this->size(), " at the offset ", offset);
        glBufferSubData(static_cast<GLenum>(T), static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(output_block.size()), output_block.begin);
    }

    template<BufferType T>
    void Buffer<T>::send(const void* output_data)
    {
        IBuffer::verify();
        this->verify_bound();
        if(!this->is_terminal())
            topaz_assert(!this->is_mapped(), "tz::gl::Buffer::send(void*): Cannot send because this buffer is both non-terminal and mapped. Cannot send data to a non-terminal buffer if it is mapped.");
        // This is alot less safe for obvious reasons. But it's very low overhead so we should support it.
        glBufferSubData(static_cast<GLenum>(T), 0, static_cast<GLsizeiptr>(this->size()), output_data);
    }

    template<BufferType T>
    void Buffer<T>::terminal_resize(std::size_t size_bytes)
    {
        IBuffer::verify();
        this->verify_bound();
        IBuffer::verify_nonterminal();
        topaz_assert(!this->is_mapped(), "tz::gl::Buffer<T>::terminal_resize(", size_bytes, "): Cannot resize because this buffer is currently mapped.");
        glBufferStorage(static_cast<GLenum>(T), size_bytes, nullptr, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    }

    template<BufferType T>
    void Buffer<T>::make_terminal()
    {
        IBuffer::verify();
        this->verify_bound();
        IBuffer::verify_nonterminal();
        topaz_assert(!this->is_mapped(), "tz::gl::Buffer<T>::make_terminal(): Cannot make terminal because the buffer is currently mapped.");
        // TODO: Maintain a copy of the underlying data first and copy that data back into the immutable data store.
        glBufferStorage(static_cast<GLenum>(T), this->size(), nullptr, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    }

    template<BufferType T>
    tz::mem::Block Buffer<T>::map(MappingPurpose purpose)
    {
        IBuffer::verify();
        this->verify_bound();
        topaz_assert(!this->is_mapped(), "tz::gl::Buffer<T>::map(...): Attempted to map but we are already mapped");
        // We know for sure that we have a valid handle, it is currently bound and we're definitely not yet mapped.
        void* begin = nullptr;
        if(this->is_terminal())
            begin = glMapBufferRange(static_cast<GLenum>(T), 0, this->size(), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
        else
            begin = glMapBuffer(static_cast<GLenum>(T), static_cast<GLenum>(purpose));

        return {begin, this->size()};
    }

    template<BufferType T>
    void Buffer<T>::unmap()
    {
        IBuffer::verify();
        this->verify_bound();
        topaz_assert(this->is_mapped(), "tz::gl::Buffer<T>::unmap(): Attempted to unmap but we weren't already mapped");
        // We know for sure that we have a valid handle, it is currently bound and we're definitely mapped.
        glUnmapBuffer(static_cast<GLenum>(T));
    }

    template<BufferType T>
    bool Buffer<T>::is_mapped() const
    {
        IBuffer::verify();
        this->verify_bound();
        GLint param;
        glGetBufferParameteriv(static_cast<GLenum>(T), GL_BUFFER_MAPPED, &param);
        return param == GL_TRUE ? true : false;
    }

    template<BufferType T>
    void Buffer<T>::verify_bound() const
    {
        topaz_assert(this->operator==(tz::gl::bound::buffer(T)), "tz::gl::Buffer<T>::verify_bound(): Verification Failed!");
    }
}