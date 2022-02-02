#ifndef TOPAZ_GL_IMPL_BACKEND_OGL2_BUFFER_HPP
#define TOPAZ_GL_IMPL_BACKEND_OGL2_BUFFER_HPP
#if TZ_OGL
#include "gl/impl/backend/ogl2/tz_opengl.hpp"

namespace tz::gl::ogl2
{
	/**
	 * @ingroup tz_gl_ogl2_buffers
	 * Specifies the target to which the buffer is bound for its data store.
	 */
	enum class BufferTarget : GLenum
	{
		/// - Uniform Buffers (UBOs).
		Uniform = GL_UNIFORM_BUFFER,
		/// - Shader Storage Buffers (SSBOs).
		ShaderStorage = GL_SHADER_STORAGE_BUFFER
	};

	/**
	 * @ingroup tz_gl_ogl2_buffers
	 * Specifies the expected behaviour of the data store contents.
	 */
	enum class BufferResidency
	{
		/// - The data store contents can never be modified except from by transfer commands. This means that it cannot be mapped at all.
		Static,
		/// - The data store contents could be modified repeatedly. The buffer is read+write mappable.
		Dynamic
	};

	/**
	 * @ingroup tz_gl_ogl2_buffers
	 * Specifies creation flags for a @ref Buffer
	 */
	struct BufferInfo
	{
		/// Specifies the target to which the buffer is bound. Only one buffer can be bound to a target at a time.
		BufferTarget target;
		/// Describes the usage and behaviour of the buffer's data store.
		BufferResidency residency;
		/// Specifies the size of the buffer, in bytes. Buffers cannot be resized.
		std::size_t size_bytes;
	};

	/**
	 * @ingroup tz_gl_ogl2_buffers
	 * Documentation for OpenGL Buffers.
	 */
	class Buffer
	{
	public:
		/**
		 * Create a new Buffer.
		 * @param info Specifies creation flags for the buffer.
		 */
		Buffer(BufferInfo info);
		Buffer(const Buffer& copy) = delete;
		Buffer(Buffer&& move);
		~Buffer();
		Buffer& operator=(const Buffer& copy) = delete;
		Buffer& operator=(Buffer&& move);

		/**
		 * Retrieves the target to which the buffer is bound.
		 * @return Buffer bind target.
		 */
		BufferTarget get_target() const;
		/**
		 * Retrieves the residency of the buffer.
		 * @return Buffer residency.
		 */
		BufferResidency get_residency() const;
		/**
		 * Retrieve the size of the buffer's data-store, in bytes.
		 */
		std::size_t size() const;
		/**
		 * Map the buffer, receiving a CPU-visible pointer.
		 * @pre The buffer must have been created with a dynamic residency. See @ref BufferResidency::Dynamic. Otherwise, the behaviour is undefined.
		 * @return Pointer to a block of memory of size `this->size()` bytes.
		 */
		void* map();
		/**
		 * Map the buffer as an array of some type.
		 * @pre The buffer must have been created with a dynamic residency. See @ref BufferResidency::Dynamic. Otherwise, the behaviour is undefined.
		 * @tparam T Type of which to retrieve an array of.
		 * @return Span representing a view into an array of the provided type.
		 */
		template<typename T>
		std::span<T> map_as()
		{
			return {reinterpret_cast<T*>(this->map(), this->size() / sizeof(T))};
		}

		using NativeType = GLuint;
		NativeType native() const;
	private:
		GLuint buffer;
		BufferInfo info;
		void* mapped_ptr = nullptr;
	};
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_BACKEND_OGL2_BUFFER_HPP
