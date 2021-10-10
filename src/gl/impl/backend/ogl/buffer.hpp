#ifndef TOPAZ_GL_OGL_BUFFER_HPP
#define TOPAZ_GL_OGL_BUFFER_HPP
#if TZ_OGL
#include "glad/glad.h"
#include <cstdint>

namespace tz::gl::ogl
{
	enum class BufferType
	{
		Vertex,
		Index,
		Uniform,
		ShaderStorage,
		DrawIndirect
	};

	enum class BufferPurpose : GLenum
	{
		StaticDraw = GL_STATIC_DRAW,
		StaticRead = GL_STATIC_READ,
		StaticCopy = GL_STATIC_COPY,
		DynamicDraw = GL_DYNAMIC_DRAW,
		DynamicRead = GL_DYNAMIC_READ,
		DynamicCopy = GL_DYNAMIC_COPY,
		StreamDraw = GL_STREAM_DRAW,
		StreamRead = GL_STREAM_READ,
		SteamCopy = GL_STREAM_COPY
	};

	enum class BufferUsage
	{
		ReadWrite,
		PersistentMapped
	};

	class Buffer
	{
	public:
		Buffer(BufferType type, BufferPurpose purpose, BufferUsage usage, std::size_t bytes);
		Buffer(const Buffer& copy) = delete;
		Buffer(Buffer&& move);
		~Buffer();
		Buffer& operator=(const Buffer& rhs) = delete;
		Buffer& operator=(Buffer&& rhs);
	
		BufferType get_type() const;

		void write(const void* addr, std::size_t bytes);
		void* map_memory();
		void unmap_memory();
		void bind() const;
		void bind_base(GLuint index) const;

		GLuint native() const;

		bool is_null() const;
		static Buffer null();
	private:
		Buffer();
		
		GLuint buf;
		BufferType type;
		BufferUsage usage;
		void* mapped_ptr;
	};
}

#endif // TZ_OGL
#endif // TOPAZ_GL_OGL_BUFFER_HPP