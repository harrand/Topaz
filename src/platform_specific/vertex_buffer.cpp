//
// Created by Harrand on 04/04/2019.
//

#include "vertex_buffer.hpp"

#ifdef TOPAZ_OPENGL
namespace tz::platform
{
	OGLVertexBuffer::OGLVertexBuffer(): OGLGenericBuffer<OGLBufferType::ARRAY>(){}

	OGLVertexBuffer::OGLVertexBuffer(const OGLVertexBuffer& copy): OGLVertexBuffer()
	{
		std::optional<std::vector<std::byte>> generic_data = copy.query_all_data<std::vector, std::byte>();
		std::optional<tz::platform::OGLBufferUsage> generic_usage = copy.query_current_usage();
		if(generic_data.has_value() && generic_usage.has_value())
		{
			this->insert(generic_data.value(), generic_usage.value());
		}
	}

	OGLIndexBuffer::OGLIndexBuffer(): OGLGenericBuffer<OGLBufferType::INDEX>(){}

	OGLIndexBuffer::OGLIndexBuffer(const tz::platform::OGLIndexBuffer& copy): OGLGenericBuffer<OGLBufferType::INDEX>(copy){}

	OGLVertexTransformFeedbackBuffer::OGLVertexTransformFeedbackBuffer(GLuint output_id): OGLGenericBuffer<OGLBufferType::TRANSFORM_FEEDBACK>(), output_id(output_id){}

	void OGLVertexTransformFeedbackBuffer::bind() const
	{
		OGLGenericBuffer<OGLBufferType::TRANSFORM_FEEDBACK>::bind();
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, this->output_id, this->buffer_handle);
	}

	OGLVertexAttribute::OGLVertexAttribute(GLuint attribute_id): attribute_id(attribute_id){}

	GLuint OGLVertexAttribute::get_id() const
	{
		return this->attribute_id;
	}

	void OGLVertexAttribute::enable() const
	{
		glEnableVertexAttribArray(this->attribute_id);
	}

	void OGLVertexAttribute::disable() const
	{
		glDisableVertexAttribArray(this->attribute_id);
	}

	void OGLVertexAttribute::direct_define(GLint size, const OGLVertexAttributeType& type, GLboolean normalised, GLsizei stride) const
	{
		glVertexAttribPointer(this->attribute_id, size, static_cast<GLenum>(type), normalised, stride, NULL);
	}

	void OGLVertexAttribute::instanced_define(GLuint divisor) const
	{
		glVertexAttribDivisor(this->attribute_id, divisor);
	}

	OGLVertexArray::OGLVertexArray(): vao_handle(0), vertex_buffers(), index_buffer(nullptr), vertex_attributes()
	{
		glGenVertexArrays(1, &this->vao_handle);
	}

	OGLVertexArray::OGLVertexArray(const OGLVertexArray& copy): OGLVertexArray()
	{
		// Emplace copies of all attributes and buffers.
		for(const std::unique_ptr<OGLVertexAttribute>& attrib_ptr : copy.vertex_attributes)
			this->emplace_vertex_attribute(*attrib_ptr);
		if(copy.index_buffer != nullptr)
			this->index_buffer = std::make_unique<OGLIndexBuffer>(*copy.index_buffer);
		for(const std::unique_ptr<OGLVertexBuffer>& buffer_ptr : copy.vertex_buffers)
			this->emplace_vertex_buffer(*buffer_ptr);
	}

	OGLVertexArray::OGLVertexArray(OGLVertexArray&& move): vao_handle(move.vao_handle), vertex_buffers(std::move(move.vertex_buffers)), index_buffer(std::move(move.index_buffer)), vertex_attributes(std::move(move.vertex_attributes))
	{
		move.vao_handle = 0;
	}

	OGLVertexArray::~OGLVertexArray()
	{
		// "Unused names in arrays are silently ignored, as is the value zero." - OGL Specification
		glDeleteVertexArrays(1, &this->vao_handle);
	}

	OGLVertexArray& OGLVertexArray::operator=(OGLVertexArray rhs)
	{
		std::swap(*this, rhs);
		return *this;
	}

	void OGLVertexArray::bind() const
	{
		glBindVertexArray(this->vao_handle);
	}

	void OGLVertexArray::unbind() const
	{
		glBindVertexArray(0);
	}

	const OGLVertexAttribute* OGLVertexArray::get_attribute(GLuint attribute_id) const
	{
		for(const std::unique_ptr<OGLVertexAttribute>& attrib_ptr : this->vertex_attributes)
			if(attrib_ptr->get_id() == attribute_id)
				return attrib_ptr.get();
		return nullptr;
	}

	const OGLIndexBuffer* OGLVertexArray::get_element_array_buffer() const
	{
		return this->index_buffer.get();
	}

	bool OGLVertexArray::operator==(const OGLVertexArray& rhs) const
	{
		return this->vao_handle == rhs.vao_handle;
	}

	void OGLVertexArray::swap(OGLVertexArray& lhs, OGLVertexArray& rhs)
	{
		std::swap(lhs.vao_handle, rhs.vao_handle);
		std::swap(lhs.vertex_buffers, rhs.vertex_buffers);
		std::swap(lhs.index_buffer, rhs.index_buffer);
		std::swap(lhs.vertex_attributes, rhs.vertex_attributes);
	}
}
#endif