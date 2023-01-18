#ifndef TOPAZ_GL_IMPL_BACKEND_OGL2_BUFFER_HPP
#define TOPAZ_GL_IMPL_BACKEND_OGL2_BUFFER_HPP
#if TZ_OGL
#include "tz/gl/impl/opengl/detail/tz_opengl.hpp"

namespace tz::gl::ogl2
{
	/**
	 * @ingroup tz_gl_ogl2_buffers
	 * Specifies the target to which the buffer is bound for its data store.
	 */
	enum class BufferTarget : GLenum
	{
		/// - Index Buffers (IBOs).
		Index = GL_ELEMENT_ARRAY_BUFFER,
		/// - Draw Indirect Buffers.
		DrawIndirect = GL_DRAW_INDIRECT_BUFFER,
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
		 * Map the buffer, receiving a CPU-visible pointer. Read-only.
		 * @pre The buffer must have been created with a dynamic residency. See @ref BufferResidency::Dynamic. Otherwise, the behaviour is undefined.
		 * @return Pointer to a block of memory of size `this->size()` bytes.
		 */
		const void* map() const;
		/**
		 * Unmap the buffer.
		 * @pre The buffer must have previously been mapped via `map()` or equivalent. Otherwise, the behaviour is undefined.
		 */
		void unmap();
		/**
		 * Map the buffer as an array of some type.
		 * @pre The buffer must have been created with a dynamic residency. See @ref BufferResidency::Dynamic. Otherwise, the behaviour is undefined.
		 * @tparam T Type of which to retrieve an array of.
		 * @return Span representing a view into an array of the provided type.
		 */
		template<typename T>
		std::span<T> map_as()
		{
			return {reinterpret_cast<T*>(this->map()), this->size() / sizeof(T)};
		}
		/**
		 * Map the buffer as an array of some type. Read-only.
		 * @pre The buffer must have been created with a dynamic residency. See @ref BufferResidency::Dynamic. Otherwise, the behaviour is undefined.
		 * @tparam T Type of which to retrieve an array of.
		 * @return Span representing a view into an array of the provided type.
		 */
		template<tz::const_type T>
		std::span<T> map_as() const
		{
			return {reinterpret_cast<T*>(this->map()), this->size() / sizeof(T)};
		}
		/**
		 * Bind without a resource id (i.e you are an Index Buffer).
		 */
		void basic_bind() const;
		void custom_bind(BufferTarget tar) const;
		/**
		 * Bind the buffer to a shader resource id.
		 */
		void bind_to_resource_id(unsigned int shader_resource_id) const;
		/**
		 * Create a buffer which acts as a null buffer, that is, no operations are valid on it.
		 * @return Null Buffer.
		 */
		static Buffer null();
		/**
		 * Query as to whether the buffer is a null buffer. A null buffer is equivalent to @ref Buffer::null().
		 */
		bool is_null() const;

		using NativeType = GLuint;
		NativeType native() const;
		
		std::string debug_get_name() const;
		void debug_set_name(std::string name);
	private:
		Buffer();

		GLuint buffer;
		BufferInfo info;
		mutable void* mapped_ptr = nullptr;
		std::string debug_name = "";
	};

	namespace buffer
	{
		/**
	 	 * @ingroup tz_gl_ogl2_buffers
		 * Copy the entire data store of a buffer to another. If the destination buffer is larger than the source buffer, the bytes following the copy region are untouched.
		 * @pre The destination buffer must have size greater than or equal to the source buffer, otherwise the behaviour is undefined.
		 */
		void copy(const Buffer& source, Buffer& destination);
		/**
	 	 * @ingroup tz_gl_ogl2_buffers
		 * Buffers are not resizeable. However, this function creates an exact copy of the buffer, but with a new size. You can assume that the target, residency and data store contents are completely unchanged. If the resize increases the size of the buffer however, then the new bytes have undefined values.
		 */
		Buffer clone_resized(const Buffer& buf, std::size_t new_size);
	}
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_BACKEND_OGL2_BUFFER_HPP
