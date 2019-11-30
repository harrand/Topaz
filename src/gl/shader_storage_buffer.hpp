//
// Created by Harry on 27/05/2019.
//

#ifndef TOPAZ_SHADER_STORAGE_BUFFER_HPP
#define TOPAZ_SHADER_STORAGE_BUFFER_HPP
#include "gl/generic_buffer.hpp"

namespace tz::gl
{
	class OGLShaderStorageBuffer : public OGLGenericBuffer<OGLBufferType::SHADER_STORAGE>
	{
	public:
		OGLShaderStorageBuffer(GLuint layout_qualifier_id);
		virtual void bind() const override;
		friend class OGLShaderProgram;
	private:
		GLuint layout_qualifier_id;
	};
}


#endif //TOPAZ_SHADER_STORAGE_BUFFER_HPP
