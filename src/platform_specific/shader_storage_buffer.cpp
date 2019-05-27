//
// Created by Harry on 27/05/2019.
//

#include "shader_storage_buffer.hpp"

namespace tz::platform
{
	OGLShaderStorageBuffer::OGLShaderStorageBuffer(GLuint layout_qualifier_id) : OGLGenericBuffer<OGLBufferType::SHADER_STORAGE>(), layout_qualifier_id(layout_qualifier_id) {}

	void OGLShaderStorageBuffer::bind() const
	{
		OGLGenericBuffer<OGLBufferType::SHADER_STORAGE>::bind();
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, this->layout_qualifier_id, this->buffer_handle);
	}
}