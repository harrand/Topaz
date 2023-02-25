#if TZ_OGL
#include "tz/core/profile.hpp"
#include "tz/gl/impl/opengl/detail/buffer.hpp"

namespace tz::gl::ogl2
{
	buffer::buffer(buffer_info info):
	buf(0),
	info(info)
	{
		TZ_PROFZONE("ogl - buffer create", 0xFFAA0000);
		tz::assert(ogl2::is_initialised(), "Cannot create ogl2 buffer because ogl2 backend has not yet been initialised! Please submit a bug report.");
		tz::assert(this->info.size_bytes > 0, "Cannot create a zero-sized buffer.");
		glCreateBuffers(1, &this->buf);
		constexpr GLenum buffer_create_flags_static = 0;
		constexpr GLenum buffer_create_flags_dynamic = GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		GLenum flags = buffer_create_flags_static;
		if(this->info.residency == buffer_residency::dynamic)
		{
			flags = buffer_create_flags_dynamic;
		}
		glNamedBufferStorage(this->buf, static_cast<GLsizeiptr>(this->info.size_bytes), nullptr, flags);
	}

	buffer::buffer(buffer&& move):
	buf(0),
	info()
	{
		*this = std::move(move);
	}

	buffer::~buffer()
	{
		TZ_PROFZONE("ogl - buffer destroy", 0xFFAA0000);
		glDeleteBuffers(1, &this->buf);
	}

	buffer& buffer::operator=(buffer&& move)
	{
		std::swap(this->buf, move.buf);
		std::swap(this->info, move.info);
		std::swap(this->mapped_ptr, move.mapped_ptr);
		return *this;
	}

	buffer_target buffer::get_target() const
	{
		return this->info.target;
	}

	buffer_residency buffer::get_residency() const
	{
		return this->info.residency;
	}

	std::size_t buffer::size() const
	{
		return this->info.size_bytes;
	}

	void* buffer::map()
	{
		TZ_PROFZONE("ogl - buffer map", 0xFFAA0000);
		if(this->mapped_ptr != nullptr)
		{
			return this->mapped_ptr;
		}

		tz::assert(this->info.residency == buffer_residency::dynamic, "OGL buffers with non-dynamic residency cannot be mapped. Please submit a bug report.");
		this->mapped_ptr = glMapNamedBufferRange(this->buf, 0, static_cast<GLsizeiptr>(this->size()), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
		tz::assert(this->mapped_ptr != nullptr, "buffer::map() returned nullptr. Please submit a bug report.");
		return this->mapped_ptr;
	}

	const void* buffer::map() const
	{
		TZ_PROFZONE("ogl - buffer map", 0xFFAA0000);
		if(this->mapped_ptr != nullptr)
		{
			return this->mapped_ptr;
		}

		tz::assert(this->info.residency == buffer_residency::dynamic, "OGL buffers with non-dynamic residency cannot be mapped. Please submit a bug report.");
		this->mapped_ptr = glMapNamedBufferRange(this->buf, 0, static_cast<GLsizeiptr>(this->size()), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
		tz::assert(this->mapped_ptr != nullptr, "buffer::map() returned nullptr. Please submit a bug report.");
		return this->mapped_ptr;
	}

	void buffer::unmap()
	{
		TZ_PROFZONE("ogl - buffer unmap", 0xFFAA0000);
		this->mapped_ptr = nullptr;
		glUnmapNamedBuffer(this->native());
	}

	void buffer::basic_bind() const
	{
		TZ_PROFZONE("ogl - buffer basic bind", 0xFFAA0000);
		this->custom_bind(this->get_target());
	}

	void buffer::custom_bind(buffer_target tar) const
	{
		TZ_PROFZONE("ogl - buffer custom bind", 0xFFAA0000);
		glBindBuffer(static_cast<GLenum>(tar), this->buf);
	}

	void buffer::bind_to_resource_id(unsigned int shader_resource_id) const
	{
		TZ_PROFZONE("ogl - buffer bind to resource", 0xFFAA0000);
		tz::assert(this->get_target() == buffer_target::uniform || this->get_target() == buffer_target::shader_storage, "Attempted to bind buffer to resource id %u, but its target was invalid - Only UBOs and SSBOs can be bound to a resource id.", shader_resource_id);
		glBindBufferBase(static_cast<GLenum>(this->get_target()), shader_resource_id, this->buf);
	}

	buffer buffer::null()
	{
		return {};
	}

	bool buffer::is_null() const
	{
		return this->buf == 0;
	}

	buffer::NativeType buffer::native() const
	{
		return this->buf;
	}

	std::string buffer::debug_get_name() const
	{
		return this->debug_name;
	}

	void buffer::debug_set_name(std::string name)
	{
		this->debug_name = name;
		#if TZ_DEBUG
			glObjectLabel(GL_BUFFER, this->buf, -1, this->debug_name.c_str());
		#endif
	}

	buffer::buffer():
	buf(0),
	info(){}

	namespace buffer_helper
	{
		void copy(const buffer& source, buffer& destination)
		{
			TZ_PROFZONE("ogl - buffer copy operation", 0xFFAA0000);

			tz::assert(destination.size() >= source.size(), "buffer Copy: buffer source was larger than destination; therefore destination does not have enough space for transfer. Please submit a bug report.");
			glCopyNamedBufferSubData(source.native(), destination.native(), 0, 0, static_cast<GLsizeiptr>(source.size()));
		}

		buffer clone_resized(const buffer& buf, std::size_t new_size)
		{
			TZ_PROFZONE("ogl - buffer clone operation", 0xFFAA0000);
			buffer newbuf
			{{
				.target = buf.get_target(),
				.residency = buf.get_residency(),
				.size_bytes = new_size
			}};
			if(new_size <= buf.size())
			{

				// We're smaller, so we copy all that we can.
				glCopyNamedBufferSubData(buf.native(), newbuf.native(), 0, 0, static_cast<GLsizeiptr>(new_size));
			}
			else
			{
				// We're bigger or same size, so just copy the entire source. If anything is left in the destination it will be undefined values.
				copy(buf, newbuf);
			}
			return newbuf;
		}
	}
}

#endif // TZ_OGL
