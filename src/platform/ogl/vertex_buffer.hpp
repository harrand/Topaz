//
// Created by Harrand on 04/04/2019.
//

#ifndef TOPAZ_VERTEX_BUFFER_HPP
#define TOPAZ_VERTEX_BUFFER_HPP

#include "platform/ogl/generic_buffer.hpp"

#ifdef TOPAZ_OPENGL
namespace tz::gl
{
	using OGLVertexBuffer = OGLGenericBuffer<OGLBufferType::ARRAY>;
	using OGLIndexBuffer = OGLGenericBuffer<OGLBufferType::INDEX>;

	/**
	 * Like OGLVertexBuffer, but has additional functionality to support transform feedback.
	 */
	class OGLVertexTransformFeedbackBuffer : public OGLGenericBuffer<OGLBufferType::TRANSFORM_FEEDBACK>
	{
	public:
		/**
		 * Construct an empty VBO with a given target.
		 * @param output_id - Expected output ID of the corresponding variable from the fragment shader.
		 */
		OGLVertexTransformFeedbackBuffer(GLuint output_id);
		virtual ~OGLVertexTransformFeedbackBuffer() = default;
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
		template<typename... Args>
		OGLIndexBuffer& emplace_index_buffer(Args&&... args);
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
		const OGLIndexBuffer* get_element_array_buffer() const;
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
		friend class OGLMeshBuffer;
	private:
		/// Underlying OpenGL VAO handle.
		GLuint vao_handle;
		/// Storage of all VertexBuffers.
		std::vector<std::unique_ptr<OGLVertexBuffer>> vertex_buffers;
		/// Stores the IndexBuffer, if there is one.
		std::unique_ptr<OGLIndexBuffer> index_buffer;
		/// Storage of all VertexAttributes.
		std::vector<std::unique_ptr<OGLVertexAttribute>> vertex_attributes;
	};
}
#endif

#include "vertex_buffer.inl"
#endif //TOPAZ_VERTEX_BUFFER_HPP