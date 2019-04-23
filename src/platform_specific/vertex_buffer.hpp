//
// Created by Harrand on 04/04/2019.
//

#ifndef TOPAZ_VERTEX_BUFFER_HPP
#define TOPAZ_VERTEX_BUFFER_HPP

#include "core/topaz.hpp"

#ifdef TOPAZ_OPENGL
namespace tz::platform
{
	/**
	 * Describes how the VertexBuffer should be interpreted by OpenGL drivers.
	 */
	enum class OGLVertexBufferTarget : GLenum
	{
		ARRAY = GL_ARRAY_BUFFER,
		ELEMENT_ARRAY = GL_ELEMENT_ARRAY_BUFFER
	};

	/**
	 * Describes how often the VertexBuffer is expected to be used.
	 */
	enum class OGLVertexBufferFrequency
	{
		STREAM, STATIC, DYNAMIC
	};

	/**
	 * Describes in which way the VertexBuffer is expected to be used.
	 */
	enum class OGLVertexBufferNature
	{
		DRAW, READ, COPY
	};

	/**
	 * Describes the usage of a given VertexBuffer for OpenGL.
	 */
	class OGLVertexBufferUsage
	{
	public:
		/**
		 * Define the usage of a given VertexBuffer.
		 * @param frequency - How often the buffer is expected to be used?
		 * @param nature - How is it going to be used?
		 */
		OGLVertexBufferUsage(OGLVertexBufferFrequency frequency, OGLVertexBufferNature nature);
		friend class OGLVertexBuffer;
	private:
		/**
		 * Construct a usage based upon an existing OpenGL usage, such as GL_STATIC_DRAW. If the enum isn't a valid usage, the usage defaults to GL_STATIC_DRAW.
		 * @param usage - The usage to decipher
		 */
		OGLVertexBufferUsage(const GLenum& usage);
		/**
		 * Obtain the OpenGL enumeration equivalent to the description of the given VertexBuffer.
		 * @return - OpenGL applicable usage enumeration, such as GL_STATIC_DRAW
		 */
		GLenum operator()() const;

		/// Underlying frequency.
		OGLVertexBufferFrequency frequency;
		/// Underlying nature.
		OGLVertexBufferNature nature;
	};

	/**
	 * Wrapper for an OpenGL vertex-buffer-object (VBO)
	 */
	class OGLVertexBuffer
	{
	public:
		/**
		 * Construct an empty VBO with a given target.
		 * @param target - Target describing how this VBO is to be interpreted by OpenGL
		 */
		OGLVertexBuffer(OGLVertexBufferTarget target);
		/**
		 * Deep-copy from an existing OGLVertexBuffer. This means that it shall not be attached to the same OGLVertexArray unless done so via OGLVertexArray::emplace_vertex_buffer(...).
		 * @param copy - The OGLVertexBuffer to copy from
		 */
		OGLVertexBuffer(const OGLVertexBuffer& copy);
		/**
		 * Move from an existing OGLVertexBuffer. No VRAM allocation takes place.
		 * @param move -
		 */
		OGLVertexBuffer(OGLVertexBuffer&& move);
		/**
		 * Ensure that the VBO is freed in both VRAM and RAM.
		 */
		virtual ~OGLVertexBuffer();
		/**
		 * Deep-copy using the copy-and-swap idiom. The current data is discarded.
		 * @param rhs - OGLVertexBuffer to copy data from.
		 * @return - The edited OGLVertexBuffer
		 */
		OGLVertexBuffer& operator=(OGLVertexBuffer rhs);
		/**
		 * Get the description of the target of this VBO.
		 * @return - Target describing how this VBO is to be interpreted by OpenGL
		 */
		const OGLVertexBufferTarget& get_target() const;
		/**
		 * Obtain the size of the VBO in VRAM, in bytes.
		 * @return - Size in bytes
		 */
		std::size_t get_size() const;
		/**
		 * Query as to whether this VBO is empty or not.
		 * @return - True if the size is 0B, false otherwise
		 */
		bool empty() const;
		/**
		 * Allocate VRAM specifically for this VBO. The allocated memory will be zeroed.
		 * @param size - Size in bytes for the memory allocation
		 * @param usage - Describe the usage of this memory allocation, for optimisation reasons
		 */
		void allocate_memory(std::size_t size, const OGLVertexBufferUsage& usage) const;
		/**
		 * Allocate enough VRAM to contain a given container of POD (Plain-old-data), and then fill it accordingly.
		 * @tparam Container - Type of the container, such as std::vector
		 * @tparam POD - Type of an element of the data payload, such as std::array<float, 3> or double
		 * @param data - Container holding the data payload to be uploaded to VRAM
		 * @param usage - Describe the usage of the underlying memory allocation, for optimisation reasons
		 */
		template<template<typename> typename Container, typename POD>
		void insert(const Container<POD>& data, const OGLVertexBufferUsage& usage) const;
		/**
		 * Re-allocate this VertexBuffer memory pool and fill it with the given data, if any.
		 * @param offset - Specifies the offset into the buffer object's data store where data replacement will begin, in bytes
		 * @param size - Specifies the size, in bytes, of the data store region being replaced
		 * @param data - Data to be copied into the new storage. If nullptr is passed, the storage will be zeroed
		 */
		void update(GLintptr offset, GLsizeiptr size, const void* data) const;
		template<template<typename> typename Container, typename POD>
		/**
		 * Retrieve all allocated memory and its corresponding data, and organise it into a given container of POD (Plain-old-data).
		 * @tparam Container - Type of the container, such as std::vector
		 * @tparam POD - Type of an element of the data payload, such as std::array<float, 3> or double
		 * @return - Container holding a copy of the data payload currently stored in VRAM
		 */
		std::optional<Container<POD>> query_all_data() const;
		/**
		 * Retrieve the usage specified when creating the currently allocated memory. Returns null if no such allocation took place previously.
		 * @return - If memory is allocated, the usage is returned. Otherwise, null is returned
		 */
		std::optional<OGLVertexBufferUsage> query_current_usage() const;
		/**
		 * Bind this VertexBuffer manually. You will likely need this if you intend to extend the functionality of this VertexBuffer.
		 */
		virtual void bind() const;
		/**
		 * Unbind this VertexBuffer manually. You will likely need this if you intend to extend the functionality of this VertexBuffer.
		 */
		virtual void unbind() const;
	protected:
		/// Trivially swap two vertex buffers.
		static void swap(OGLVertexBuffer& lhs, OGLVertexBuffer& rhs);
		/// Underlying target.
		OGLVertexBufferTarget target;
		/// Underlying OpenGL handle for the VRAM buffer.
		GLuint vbo_handle;
	};

	/**
	 * Like OGLVertexBuffer, but has additional functionality to support transform feedback.
	 */
	class OGLVertexTransformFeedbackBuffer : public OGLVertexBuffer
	{
	public:
		/**
		 * Construct an empty VBO with a given target.
		 * @param target - Target describing how this VBO is to be interpreted by OpenGL.
		 * @param output_id - Expected output ID of the corresponding variable from the fragment shader.
		 */
		OGLVertexTransformFeedbackBuffer(OGLVertexBufferTarget target, GLuint output_id);
		/**
		 * Bind this VBO manually.
		 */
		virtual void bind() const override;
	private:
		/// Stores the expected output ID from the fragment shader.
		GLuint output_id;
	};

	/**
	 * Describes the underlying type that the corresponding VBO should store.
	 */
	enum class OGLVertexAttributeType : GLenum
	{
		SHORT = GL_SHORT,
		USHORT = GL_UNSIGNED_SHORT,
		INT = GL_INT,
		UINT = GL_UNSIGNED_INT,
		FLOAT = GL_FLOAT,
		DOUBLE = GL_DOUBLE
	};

	/**
	 * Wrapper for an OpenGL vertex attribute, describing a given VertexBuffer.
	 */
	class OGLVertexAttribute
	{
	public:
		/**
		 * Create a new VertexAttribute for a given ID.
		 * @param attribute_id - The ID of this attribute
		 */
		OGLVertexAttribute(GLuint attribute_id);
		/**
		 * Get the ID of this attribute.
		 * @return - ID of the attribute
		 */
		GLuint get_id() const;
		/**
		 * Enable this VertexAttribute, causing subsequent render-calls to factor it into account when deciphering VBO data.
		 */
		void enable() const;
		/**
		 * Disable this VertexAttribute, preventing subsequent render-calls from factoring this into account when deciphering VBO data.
		 */
		void disable() const;
		/**
		 * Defines the parameters of this vertex attribute.
		 * @tparam T - Underlying type of each element
		 * @param size - Specifies the number of components per generic vertex attribute. Must be 1, 2, 3, 4
		 * @param normalised - Specifies whether fixed-point data values should be normalized when they are accessed.
		 * @param stride - Specifies the byte offset between consecutive generic vertex attributes. If stride is 0, the generic vertex attributes are understood to be tightly packed in the array
		 */
		template<typename T>
		void define(GLint size, GLboolean normalised, GLsizei stride) const;
		/**
		 * Defines the parameters of this vertex attribute.
		 * @param size - Specifies the number of components per generic vertex attribute. Must be 1, 2, 3, 4
		 * @param type - Type of each element
		 * @param normalised - Specifies whether fixed-point data values should be normalized when they are accessed.
		 * @param stride - Specifies the byte offset between consecutive generic vertex attributes. If stride is 0, the generic vertex attributes are understood to be tightly packed in the array
		 */
		void direct_define(GLint size, const OGLVertexAttributeType& type, GLboolean normalised, GLsizei stride) const;
		/**
		 * Defines the parameters of this vertex attribute, in relation to instancing.
		 * @param divisor - Specify the number of instances that will pass between updates
		 */
		void instanced_define(GLuint divisor) const;
	private:
		/// ID of this vertex attribute.
		GLuint attribute_id;
	};

	/**
	 * Wrapper for an OpenGL vertex-array-object (VAO).
	 */
	class OGLVertexArray
	{
	public:
		/**
		 * Construct an empty VertexArray.
		 */
		OGLVertexArray();
		/**
		 * Deep-copy the data from an existing VertexArray into this.
		 * @param copy - The other VertexArray to copy from
		 */
		OGLVertexArray(const OGLVertexArray& copy);
		/**
		 * Move the data from an existing VertexArray into this.
		 * @param move - The other VertexArray to move from
		 */
		OGLVertexArray(OGLVertexArray&& move);
		/**
		 * Delete all elements in the VertexArray and free any allocated VRAM.
		 */
		~OGLVertexArray();
		/**
		 * Assign this VertexArray to be equal to an existing VertexArray via a deep-copy.
		 * @param rhs - The VertexArray to copy from
		 * @return - The edited VertexArray
		 */
		OGLVertexArray& operator=(OGLVertexArray rhs);
		/**
		 * Bind the VertexArray, causing subsequent render-calls to use this VertexArray in the graphics pipeline.
		 */
		void bind() const;
		/**
		 * Unbind the VertexArray, preventing subsequent render-calls from using this VertexArray in the graphics pipeline.
		 */
		void unbind() const;
		/**
		 * Construct a new object in-place attached to this VertexArray.
		 * @tparam OGLArrayType - Type of the object to attach to this array
		 * @tparam Args - Types of the arguments used to construct the object
		 * @param args - Argument values used to construct the object
		 * @return - The created object
		 */
		template<typename OGLArrayType, typename... Args>
		OGLArrayType& emplace(Args&&... args);
		/**
		 * Construct a new vertex-buffer in-place attached to this VertexArray.
		 * @tparam Args - Types of the arguments perfectly-forwarded to the VertexBuffer constructor
		 * @param args - Argument values used to construct the VertexBuffer
		 * @return - The created VertexBuffer
		 */
		template<typename... Args>
		OGLVertexBuffer& emplace_vertex_buffer(Args&&... args);
		/**
		 * Construct a new vertex attribute in-place attached to this VertexArray.
		 * @tparam Args - Types of the arguments perfectly-forwarded to the VertexAttribute constructor
		 * @param args - Argument values used to construct the VertexAttribute
		 * @return - The created VertexAttribute
		 */
		template<typename... Args>
		OGLVertexAttribute& emplace_vertex_attribute(Args&&... args);
		/**
		 * Issue a render-call to the OpenGL drivers, drawing the stored vertices.
		 * @tparam Container - Type of the container holding all the indices
		 * @tparam Index - Type of the index, typically an unsigned int
		 * @param tessellation - True if tessellation is expected to occur in the shader pipeline, and thus require patch primitives instead of triangulated primitives
		 * @param instance_count - Number of instances expected in the VertexArray, defaulted to zero
		 * @param indices - Container holding all the indices of the vertices. If there exists a VertexBuffer with the target ELEMENT_ARRAY, then this can be omitted in-place of that
		 */
		template<template<typename> typename Container = std::vector, typename Index = unsigned int>
		void render(bool tessellation = false, GLsizei instance_count = 0, Container<Index>* indices = nullptr) const;
		/**
		 * Retrieve an attribute by ID, if it exists.
		 * @param attribute_id - ID of the desired attribute
		 * @return - Pointer to the attribute. If it doesn't exist, nullptr is returned
		 */
		const OGLVertexAttribute* get_attribute(GLuint attribute_id) const;
		/**
		 * Retrieve the element array buffer, if it exists.
		 * @return - The element array buffer, if it exists
		 */
		const OGLVertexBuffer* get_element_array_buffer() const;
		/**
		 * Equate two VertexArrays shallowly.
		 * @param rhs - The other array with which to check for quality
		 * @return - True if the VertexArrays use the exact same buffers and attribute, false otherwise
		 */
		bool operator==(const OGLVertexArray& rhs) const;
		/**
		 * Swaps two VertexArrays.
		 * @param lhs - Left hand side
		 * @param rhs - Right hand side
		 */
		static void swap(OGLVertexArray& lhs, OGLVertexArray& rhs);
	private:
		/// Underlying OpenGL VAO handle.
		GLuint vao_handle;
		/// Storage of all VertexBuffers.
		std::vector<std::unique_ptr<OGLVertexBuffer>> vertex_buffers;
		/// Storage of all VertexAttributes.
		std::vector<std::unique_ptr<OGLVertexAttribute>> vertex_attributes;
	};
}
#endif

#include "vertex_buffer.inl"
#endif //TOPAZ_VERTEX_BUFFER_HPP
