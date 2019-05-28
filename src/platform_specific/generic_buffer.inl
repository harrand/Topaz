namespace tz::platform
{
	template<OGLBufferParameter parameter>
	GLint OGLGenericBufferImplicit::get_parameter() const
	{
		GLint ret;
		glGetNamedBufferParameteriv(this->buffer_handle, static_cast<GLenum>(parameter), &ret);
		return ret;
	}

	template<template<typename> typename Container, typename POD>
	std::optional<Container<POD>> OGLGenericBufferImplicit::query_all_data() const
	{
		std::size_t size = this->get_size();
		if(size == 0) // If there is no data stored, return null.
			return std::nullopt;
		Container<POD> data;
		// Number of elements = size in bytes / bytes per object
		data.resize(size / sizeof(POD));
		glGetNamedBufferSubData(this->buffer_handle, 0, size, data.data());
		return {data};
	}

	template<OGLBufferType T>
	void OGLGenericBuffer<T>::bind() const
	{
		glBindBuffer(static_cast<GLenum>(T), this->buffer_handle);
	}

	template<OGLBufferType T>
	void OGLGenericBuffer<T>::unbind() const
	{
		glBindBuffer(static_cast<GLenum>(T), 0);
	}

	template<OGLBufferType T>
	template<template<typename> typename Container, typename POD>
	void OGLGenericBuffer<T>::insert(const Container<POD>& data, const OGLBufferUsage& usage) const
	{
		this->bind();
		std::size_t size = data.size() * ::tz::utility::generic::sizeof_element(data);
		glNamedBufferData(this->buffer_handle, size, data.data(), usage());
	}

	template<OGLBufferType T>
	template<typename POD>
	MemoryPool<POD> OGLGenericBuffer<T>::persistently_map(std::size_t pod_count, bool retrieve_current_data) const
	{
		constexpr GLenum flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		std::size_t size_bytes = pod_count * sizeof(POD);
		// If we need to retrieve current data, then copy that into the resultant memory pool just before returning.
		void* data = nullptr;
		if(retrieve_current_data && this->query_all_data<std::vector, POD>().has_value())
			data = this->query_all_data<std::vector, POD>().value().data();
		glNamedBufferStorage(this->buffer_handle, size_bytes, data, flags);
		return {glMapNamedBufferRange(this->buffer_handle, 0, size_bytes, flags), pod_count};
	}
}