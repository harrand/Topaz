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
}