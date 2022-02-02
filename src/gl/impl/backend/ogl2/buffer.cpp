#if TZ_OGL
#include "gl/impl/backend/ogl2/buffer.hpp"

namespace tz::gl::ogl2
{
	Buffer::Buffer(BufferInfo info):
	buffer(0),
	info(info)
	{
		glCreateBuffers(1, &this->buffer);
		switch(this->info.residency)
		{
			case BufferResidency::Static:
				// Cannot be written to directly via neither glBufferSubData nor mapping, but transfer commands can be performed i.e `glCopyBufferSubData`
				glNamedBufferStorage(this->buffer, static_cast<GLsizeiptr>(this->info.size_bytes), nullptr, 0);
			break;
			case BufferResidency::Dynamic:
				glNamedBufferStorage(this->buffer, static_cast<GLsizeiptr>(this->info.size_bytes), nullptr, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
			break;
			default:
				tz_error("Unrecognised ogl2::BufferResidency. Please submit a bug report.");
			break;
		}
	}

	Buffer::Buffer(Buffer&& move):
	buffer(0),
	info()
	{
		*this = std::move(move);
	}

	Buffer::~Buffer()
	{
		glDeleteBuffers(1, &this->buffer);
	}

	Buffer& Buffer::operator=(Buffer&& move)
	{
		std::swap(this->buffer, move.buffer);
		std::swap(this->info, move.info);
		return *this;
	}

	std::size_t Buffer::size() const
	{
		return this->info.size_bytes;
	}

	void* Buffer::map()
	{
		if(this->mapped_ptr != nullptr)
		{
			return this->mapped_ptr;
		}

		tz_assert(this->info.residency == BufferResidency::Dynamic, "OGL Buffers with non-dynamic residency cannot be mapped. Please submit a bug report.");
		this->mapped_ptr = glMapNamedBufferRange(this->buffer, 0, static_cast<GLsizeiptr>(this->size()), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
		tz_assert(this->mapped_ptr != nullptr, "Buffer::map() returned nullptr. Please submit a bug report.");
		return this->mapped_ptr;
	}

	Buffer::NativeType Buffer::native() const
	{
		return this->buffer;
	}
}

#endif // TZ_OGL
