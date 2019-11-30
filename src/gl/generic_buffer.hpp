//
// Created by Harry on 04/05/2019.
//

#ifndef TOPAZ_GENERIC_BUFFER_HPP
#define TOPAZ_GENERIC_BUFFER_HPP

#include "core/topaz.hpp"
#include "utility/memory.hpp"

namespace tz::gl
{
	enum class OGLBufferType : GLenum
	{
		ARRAY = GL_ARRAY_BUFFER,
		ATOMIC_COUNTER = GL_ATOMIC_COUNTER_BUFFER,
		COPY_SOURCE = GL_COPY_READ_BUFFER,
		COPY_DESTINATION = GL_COPY_WRITE_BUFFER,
		INDIRECT_COMPUTE_DISPATCH_COMMAND = GL_DISPATCH_INDIRECT_BUFFER,
		INDIRECT_COMMAND_ARGUMENT = GL_DRAW_INDIRECT_BUFFER,
		INDEX = GL_ELEMENT_ARRAY_BUFFER,
		PIXEL_READ_TARGET = GL_PIXEL_PACK_BUFFER,
		TEXTURE_DATA_SOURCE = GL_PIXEL_UNPACK_BUFFER,
		QUERY_RESULT = GL_QUERY_BUFFER,
		SHADER_STORAGE = GL_SHADER_STORAGE_BUFFER,
		TEXTURE_DATA = GL_TEXTURE_BUFFER,
		TRANSFORM_FEEDBACK = GL_TRANSFORM_FEEDBACK_BUFFER,
		UNIFORM_STORAGE = GL_UNIFORM_BUFFER,
	};

	enum class OGLBufferParameter
	{
		ACCESS = GL_BUFFER_ACCESS,
		ACCESS_FLAGS = GL_BUFFER_ACCESS_FLAGS,
		MUTABILITY = GL_BUFFER_IMMUTABLE_STORAGE,
		MAPPED = GL_BUFFER_MAPPED,
		MAP_LENGTH = GL_BUFFER_MAP_LENGTH,
		MAP_OFFSET = GL_BUFFER_MAP_OFFSET,
		SIZE = GL_BUFFER_SIZE,
		STORAGE_FLAGS = GL_BUFFER_STORAGE_FLAGS,
		USAGE = GL_BUFFER_USAGE,
	};

/**
 * Describes how the VertexBuffer should be interpreted by OpenGL drivers.
 */
	enum class OGLBufferTarget : GLenum
	{
		ARRAY = GL_ARRAY_BUFFER,
		ELEMENT_ARRAY = GL_ELEMENT_ARRAY_BUFFER
	};

/**
 * Describes how often the VertexBuffer is expected to be used.
 */
	enum class OGLBufferFrequency
	{
		STREAM, STATIC, DYNAMIC
	};

/**
 * Describes in which way the VertexBuffer is expected to be used.
 */
	enum class OGLBufferNature
	{
		DRAW, READ, COPY
	};

/**
 * Describes the usage of a given VertexBuffer for OpenGL.
 */
	class OGLBufferUsage
	{
	public:
		/**
		 * Define the usage of a given VertexBuffer.
		 * @param frequency - How often the buffer is expected to be used?
		 * @param nature - How is it going to be used?
		 */
		OGLBufferUsage(OGLBufferFrequency frequency, OGLBufferNature nature);

		template<OGLBufferType T>
		friend class OGLGenericBuffer;
		friend class OGLGenericBufferImplicit;

	private:
		/**
		 * Construct a usage based upon an existing OpenGL usage, such as GL_STATIC_DRAW. If the enum isn't a valid usage, the usage defaults to GL_STATIC_DRAW.
		 * @param usage - The usage to decipher
		 */
		OGLBufferUsage(const GLenum &usage);

		/**
		 * Obtain the OpenGL enumeration equivalent to the description of the given VertexBuffer.
		 * @return - OpenGL applicable usage enumeration, such as GL_STATIC_DRAW
		 */
		GLenum operator()() const;

		/// Underlying frequency.
		OGLBufferFrequency frequency;
		/// Underlying nature.
		OGLBufferNature nature;
	};

	class OGLGenericBufferImplicit
	{
	public:
		OGLGenericBufferImplicit();
		OGLGenericBufferImplicit(const OGLGenericBufferImplicit& copy);
		OGLGenericBufferImplicit(OGLGenericBufferImplicit&& move);
		virtual ~OGLGenericBufferImplicit();
		OGLGenericBufferImplicit& operator=(const OGLGenericBufferImplicit& rhs);
		OGLGenericBufferImplicit& operator=(OGLGenericBufferImplicit&& rhs);

		virtual void bind() const = 0;
		virtual void unbind() const = 0;
		template<OGLBufferParameter parameter>
		GLint get_parameter() const;
		bool is_mutable() const;
		unsigned int get_size() const;
		bool empty() const;
		/**
		 * Allocate VRAM specifically for this VBO. The allocated memory will be zeroed.
		 * @param size - Size in bytes for the memory allocation
		 * @param usage - Describe the usage of this memory allocation, for optimisation reasons
		 */
		void allocate_memory(std::size_t size, const OGLBufferUsage& usage) const;
		/**
		 * Re-allocate this VertexBuffer memory pool and fill it with the given data, if any.
		 * @param offset - Specifies the offset into the buffer object's data store where data replacement will begin, in bytes
		 * @param size - Specifies the size, in bytes, of the data store region being replaced
		 * @param data - Data to be copied into the new storage. If nullptr is passed, the storage will be zeroed
		 */
		void update(GLintptr offset, GLsizeiptr size, const void* data) const;
		/**
		 * Retrieve all allocated memory and its corresponding data, and organise it into a given container of POD (Plain-old-data).
		 * @tparam Container - Type of the container, such as std::vector
		 * @tparam POD - Type of an element of the data payload, such as std::array<float, 3> or double
		 * @return - Container holding a copy of the data payload currently stored in VRAM
		 */
		template<template<typename> typename Container, typename POD>
		std::optional<Container<POD>> query_all_data() const;
		/**
		 * Retrieve the usage specified when creating the currently allocated memory. Returns null if no such allocation took place previously.
		 * @return - If memory is allocated, the usage is returned. Otherwise, null is returned
		 */
		std::optional<OGLBufferUsage> query_current_usage() const;
		void unmap() const;
	protected:
		GLuint buffer_handle;
	};

	template<OGLBufferType T>
	class OGLGenericBuffer : public OGLGenericBufferImplicit
	{
	public:
		OGLGenericBuffer();
		virtual ~OGLGenericBuffer() = default;

		virtual void bind() const override;
		virtual void unbind() const override;
		/**
		 * Allocate enough VRAM to contain a given container of POD (Plain-old-data), and then fill it accordingly.
		 * @tparam Container - Type of the container, such as std::vector
		 * @tparam POD - Type of an element of the data payload, such as std::array<float, 3> or double
		 * @param data - Container holding the data payload to be uploaded to VRAM
		 * @param usage - Describe the usage of the underlying memory allocation, for optimisation reasons
		 */
		template<template<typename> typename Container, typename POD>
		void insert(const Container<POD>& data, const OGLBufferUsage& usage) const;
		/**
		 * Relevant OpenGL reference: https://www.khronos.org/opengl/wiki/Buffer_Object#Persistent_mapping
		 * Map this generic buffer persistently, wrapping the buffer data into a single MemoryPool of a given type.
		 * This is incredibly useful if you intend to store instances of only one type into this buffer.
		 * Note: It is UB to invoke either insert(...), persistently_map(...) or persistently_map_variadic(...) on this class again after this invocation.
		 * @tparam POD - Type of object to store in the buffer
		 * @param pod_count - Maximum number of objects allowed in the buffer
		 * @param retrieve_current_data - Whether previous data should be preserved. Otherwise, will overwrite all existing data
		 * @return - MemoryPool managing the buffer's memory
		 */
		template<typename POD>
		MemoryPool<POD> persistently_map(std::size_t pod_count, bool retrieve_current_data) const;
		/**
		 * Relevant OpenGL reference: https://www.khronos.org/opengl/wiki/Buffer_Object#Persistent_mapping
		 * Map this generic buffer persistently around an SVMPool of known composition.
		 * This is incredibly useful if you wish to treat this buffer essentially as a struct.
		 * Example: Say you want to store a float, an int, and then a Vector3F in a generic buffer.
		 * persistently_map_variadic<float, int, Vector3F> returns a MemoryPool which allows you to read/write from it easily.
		 * Note: It is UB to invoke either insert(...), persistently_map(...) or persistently_map_variadic(...) on this class again after this invocation.
		 * @tparam Ts - Template arguments comprising the objects in the buffer
		 * @param retrieve_current_data - Whether previous data should be preserved. Otherwise, will overwrite all existing data
		 * @return - SVMPool managing the buffer's memory
		 */
		template<typename... Ts>
		SVMPool<Ts...> persistently_map_variadic(bool retrieve_current_data) const;
		/**
		 * Relevant OpenGL reference: https://www.khronos.org/opengl/wiki/Buffer_Object#Persistent_mapping
		 * Map this generic buffer persisently around a DVMPool of any composition.
		 * This is incredibly useful if you wish to be able to easily insert whatever you want into the buffer.
		 * Consider the example for an SVMPool, but you do not know the type composition at compile time. In this case, returning a DVMPool is superior.
		 * Note: It is UB to invoke either insert(...), persistently_map(...) or persistently_map_variadic(...) on this class again after this invocation.
		 * @param byte_count - Capacity of the pool, in bytes
		 * @param retrieve_current_data - Whether previous data should be preserved. Otherwise, will overwrite all existing data
		 * @return - DVMPool managing the buffer's memory
		 */
		DVMPool persistently_map_variadic(std::size_t byte_count, bool retrieve_current_data) const;
	};
}

#include "gl/generic_buffer.inl"
#endif //TOPAZ_GENERIC_BUFFER_HPP
