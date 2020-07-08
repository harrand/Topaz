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
	tz::mem::UniformPool<T> IBuffer::map_uniform(MappingPurpose purpose)
	{
		tz::mem::Block mem_block = this->map(purpose);
		return {mem_block};
	}

	template<BufferType T>
	Buffer<T>::Buffer(): IBuffer()
	{
		glBindBuffer(static_cast<GLenum>(T), this->handle);
		glBindBuffer(static_cast<GLenum>(T), 0);
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
}