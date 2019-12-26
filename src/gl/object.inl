
namespace tz::gl
{
    template<tz::gl::BufferType Type, typename... Args>
    std::size_t Object::emplace_buffer(Args&&... args)
    {
        auto buffer_ptr = std::make_unique<tz::gl::Buffer<Type>>(std::forward<Args>(args)...);
        return this->add_buffer(std::move(buffer_ptr));
    }

    template<tz::gl::BufferType Type>
    tz::gl::Buffer<Type>* Object::get(std::size_t idx)
    {
        return static_cast<tz::gl::Buffer<Type>*>((*this)[idx]);
    }

    template<tz::gl::BufferType Type>
    const tz::gl::Buffer<Type>* Object::get(std::size_t idx) const
    {
        return static_cast<const tz::gl::Buffer<Type>*>((*this)[idx]);
    }
}