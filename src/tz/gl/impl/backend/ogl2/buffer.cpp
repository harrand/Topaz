#if TZ_OGL
#include "tz/core/profiling/zone.hpp"
#include "tz/gl/impl/backend/ogl2/buffer.hpp"

namespace tz::gl::ogl2
{
	Buffer::Buffer(BufferInfo info):
	buffer(0),
	info(info)
	{
		TZ_PROFZONE("OpenGL Backend - Buffer Create", TZ_PROFCOL_RED);
		TZ_PROFZONE_GPU("Buffer Create", TZ_PROFCOL_RED);
		hdk::assert(ogl2::is_initialised(), "Cannot create ogl2 Buffer because ogl2 backend has not yet been initialised! Please submit a bug report.");
		hdk::assert(this->info.size_bytes > 0, "Cannot create a zero-sized buffer.");
		glCreateBuffers(1, &this->buffer);
		constexpr GLenum buffer_create_flags_static = 0;
		constexpr GLenum buffer_create_flags_dynamic = GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		GLenum flags = buffer_create_flags_static;
		if(this->info.residency == BufferResidency::Dynamic)
		{
			flags = buffer_create_flags_dynamic;
		}
		glNamedBufferStorage(this->buffer, static_cast<GLsizeiptr>(this->info.size_bytes), nullptr, flags);
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
		std::swap(this->mapped_ptr, move.mapped_ptr);
		return *this;
	}

	BufferTarget Buffer::get_target() const
	{
		return this->info.target;
	}

	BufferResidency Buffer::get_residency() const
	{
		return this->info.residency;
	}

	std::size_t Buffer::size() const
	{
		return this->info.size_bytes;
	}

	void* Buffer::map()
	{
		TZ_PROFZONE("OpenGL Backend - Buffer Map", TZ_PROFCOL_RED);
		TZ_PROFZONE_GPU("Buffer Map", TZ_PROFCOL_RED);
		if(this->mapped_ptr != nullptr)
		{
			return this->mapped_ptr;
		}

		hdk::assert(this->info.residency == BufferResidency::Dynamic, "OGL Buffers with non-dynamic residency cannot be mapped. Please submit a bug report.");
		this->mapped_ptr = glMapNamedBufferRange(this->buffer, 0, static_cast<GLsizeiptr>(this->size()), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
		hdk::assert(this->mapped_ptr != nullptr, "Buffer::map() returned nullptr. Please submit a bug report.");
		return this->mapped_ptr;
	}

	const void* Buffer::map() const
	{
		TZ_PROFZONE("OpenGL Backend - Buffer Map", TZ_PROFCOL_RED);
		TZ_PROFZONE_GPU("Buffer Map", TZ_PROFCOL_RED);
		if(this->mapped_ptr != nullptr)
		{
			return this->mapped_ptr;
		}

		hdk::assert(this->info.residency == BufferResidency::Dynamic, "OGL Buffers with non-dynamic residency cannot be mapped. Please submit a bug report.");
		this->mapped_ptr = glMapNamedBufferRange(this->buffer, 0, static_cast<GLsizeiptr>(this->size()), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
		hdk::assert(this->mapped_ptr != nullptr, "Buffer::map() returned nullptr. Please submit a bug report.");
		return this->mapped_ptr;
	}

	void Buffer::unmap()
	{
		this->mapped_ptr = nullptr;
		glUnmapNamedBuffer(this->native());
	}

	void Buffer::basic_bind() const
	{
		auto en = static_cast<GLenum>(this->get_target());
		hdk::assert(en == GL_ELEMENT_ARRAY_BUFFER || en == GL_DRAW_INDIRECT_BUFFER, "Basic bind on a buffer is only valid for an index or draw-indirect buffer.");
		glBindBuffer(en, this->buffer);
	}

	void Buffer::bind_to_resource_id(unsigned int shader_resource_id) const
	{
		TZ_PROFZONE("OpenGL Backend - Buffer Bind", TZ_PROFCOL_RED);
		TZ_PROFZONE_GPU("Buffer Bind", TZ_PROFCOL_RED);
		hdk::assert(this->get_target() == BufferTarget::Uniform || this->get_target() == BufferTarget::ShaderStorage, "Attempted to bind buffer to resource id %u, but its target was invalid - Only UBOs and SSBOs can be bound to a resource id.", shader_resource_id);
		glBindBufferBase(static_cast<GLenum>(this->get_target()), shader_resource_id, this->buffer);
	}

	Buffer Buffer::null()
	{
		return {};
	}

	bool Buffer::is_null() const
	{
		return this->buffer == 0;
	}

	Buffer::NativeType Buffer::native() const
	{
		return this->buffer;
	}

	std::string Buffer::debug_get_name() const
	{
		return this->debug_name;
	}

	void Buffer::debug_set_name(std::string name)
	{
		this->debug_name = name;
		#if HDK_DEBUG
			glObjectLabel(GL_BUFFER, this->buffer, -1, this->debug_name.c_str());
		#endif
	}

	Buffer::Buffer():
	buffer(0),
	info(){}

	namespace buffer
	{
		void copy(const Buffer& source, Buffer& destination)
		{
			TZ_PROFZONE("OpenGL Backend - Buffer Copy", TZ_PROFCOL_RED);
			TZ_PROFZONE_GPU("Buffer Copy", TZ_PROFCOL_RED);

			hdk::assert(destination.size() >= source.size(), "Buffer Copy: Buffer source was larger than destination; therefore destination does not have enough space for transfer. Please submit a bug report.");
			glCopyNamedBufferSubData(source.native(), destination.native(), 0, 0, static_cast<GLsizeiptr>(source.size()));
		}

		Buffer clone_resized(const Buffer& buf, std::size_t new_size)
		{
			TZ_PROFZONE("OpenGL Backend - Buffer Clone Resized", TZ_PROFCOL_RED);
			TZ_PROFZONE_GPU("Buffer Clone Resized", TZ_PROFCOL_RED);
			Buffer newbuf
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