#if TZ_OGL
#include "core/profiling/zone.hpp"
#include "gl/impl/backend/ogl2/vertex_array.hpp"

namespace tz::gl::ogl2
{
	VertexArray::VertexArray():
	vao(0)
	{
		TZ_PROFZONE("OpenGL Backend - VertexArray Create", TZ_PROFCOL_RED);
		TZ_PROFZONE_GPU("VertexArray Create", TZ_PROFCOL_RED);
		tz_assert(ogl2::is_initialised(), "Tried to create vertex array because ogl2 was not initialised. Please submit a bug report.");
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
		TZ_PROFZONE("OpenGL Backend - VertexArray Bind", TZ_PROFCOL_RED);
		TZ_PROFZONE_GPU("VertexArray Bind", TZ_PROFCOL_RED);
		glBindVertexArray(this->vao);
	}

	void VertexArray::draw(unsigned int triangle_count)
	{
		TZ_PROFZONE("OpenGL Backend - VertexArray Draw", TZ_PROFCOL_RED);
		TZ_PROFZONE_GPU("VertexArray Draw", TZ_PROFCOL_RED);
		this->bind();
		glDrawArrays(GL_TRIANGLES, 0, triangle_count * 3);
	}

	VertexArray VertexArray::null()
	{
		return {nullptr};
	}

	VertexArray::VertexArray(std::nullptr_t):
	vao(0){}
}

#endif // TZ_OGL
