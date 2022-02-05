#if TZ_OGL
#include "gl/impl/backend/ogl2/vertex_array.hpp"

namespace tz::gl::ogl2
{
	VertexArray::VertexArray():
	vao(0)
	{
		glCreateVertexArrays(1, &this->vao);
	}

	VertexArray::VertexArray(VertexArray&& move):
	vao(0)
	{
		*this = std::move(move);
	}

	VertexArray::~VertexArray()
	{
		glDeleteVertexArrays(1, &this->vao);
	}

	VertexArray& VertexArray::operator=(VertexArray&& rhs)
	{
		std::swap(this->vao, rhs.vao);
		return *this;
	}

	void VertexArray::bind()
	{
		glBindVertexArray(this->vao);
	}

	VertexArray VertexArray::null()
	{
		return {nullptr};
	}

	VertexArray::VertexArray(std::nullptr_t):
	vao(0){}
}

#endif // TZ_OGL
