
namespace tz::gl
{
	template<tz::gl::BufferType Type, typename... Args>
	std::size_t Object::emplace_buffer(Args&&... args)
	{
		this->bind();
		auto buffer_ptr = std::make_unique<tz::gl::Buffer<Type>>(std::forward<Args>(args)...);
		if constexpr(Type == tz::gl::BufferType::Index)
			return this->add_index_buffer(std::move(buffer_ptr));
		else
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

	template<tz::gl::BufferType Type, typename... Args>
	void Object::set(std::size_t idx, Args&&... args)
	{
		this->verify();
		topaz_assert(idx < this->size(), "tz::gl::Object::set(", idx, ", ...): Index given to set was not in range. Given an index ", idx, " which must be less-than (", this->size(), ")");
		// TODO: Implement
		this->erase(idx);
		topaz_assert((*this)[idx] == nullptr, "tz::gl::Object::set(", idx, ", ...): Previous element at the index ", idx, " wasn't erased properly.");
		this->buffers[idx] = std::make_unique<tz::gl::Buffer<Type>>(std::forward<Args>(args)...);
	}
}