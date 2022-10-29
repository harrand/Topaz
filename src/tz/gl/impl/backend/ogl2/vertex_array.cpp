#if TZ_OGL
#include "tz/core/profiling/zone.hpp"
#include "tz/gl/impl/backend/ogl2/vertex_array.hpp"
#include "tz/gl/impl/backend/ogl2/buffer.hpp"

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

	void VertexArray::draw(unsigned int triangle_count, bool tessellation)
	{
		TZ_PROFZONE("OpenGL Backend - VertexArray Draw", TZ_PROFCOL_RED);
		TZ_PROFZONE_GPU("VertexArray Draw", TZ_PROFCOL_RED);
		this->bind();
		glDrawArrays(tessellation ? GL_PATCHES : GL_TRIANGLES, 0, triangle_count * 3);
	}

	void VertexArray::draw_indexed(unsigned int triangle_count, const Buffer& index_buffer, bool tessellation)
	{
		TZ_PROFZONE("OpenGL Backend - VertexArray DrawIndexed", TZ_PROFCOL_RED);
		TZ_PROFZONE_GPU("VertexArray DrawIndexed", TZ_PROFCOL_RED);
		tz_assert(index_buffer.get_target() == BufferTarget::Index, "Passed non-index buffer to VertexArray::draw_indexed(...). Please submit a bug report.");
		// Note: Normally basic bind (glBindBuffer) has nothing to do with the VAO. Index buffers (GL_ELEMENT_ARRAY_BUFFER) bindings are an exception - this is explicitly associated with the currently-bound VAO. That is why we need to have the index buffer passed in via param so we can bind the vao before actually trying to bind the buffer.
		this->bind();
		index_buffer.basic_bind();
		glDrawElements(tessellation ? GL_PATCHES : GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_INT, nullptr);
	}

	void VertexArray::draw_indirect(unsigned int draw_count, const Buffer& draw_indirect_buffer, bool tessellation)
	{
		TZ_PROFZONE("OpenGL Backend - VertexArray DrawIndirect", TZ_PROFCOL_RED);
		TZ_PROFZONE_GPU("VertexArray DrawIndirect", TZ_PROFCOL_RED);
		tz_assert(draw_indirect_buffer.get_target() == BufferTarget::DrawIndirect, "Passed non-draw-indirect buffer to VertexArray::draw_indirect(...). Please submit a bug report.");
		this->bind();
		draw_indirect_buffer.basic_bind();
		glMultiDrawArraysIndirect(tessellation ? GL_PATCHES : GL_TRIANGLES, nullptr, draw_count, 0);
	}

	void VertexArray::draw_indexed_indirect(unsigned int draw_count, const Buffer& index_buffer, const Buffer& draw_indirect_buffer, bool tessellation)
	{
		TZ_PROFZONE("OpenGL Backend - VertexArray DrawIndexedIndirect", TZ_PROFCOL_RED);
		TZ_PROFZONE_GPU("VertexArray DrawIndexedIndirect", TZ_PROFCOL_RED);
		tz_assert(index_buffer.get_target() == BufferTarget::Index, "Passed non-index buffer to VertexArray::draw_indexed_indirect(...). Please submit a bug report.");
		tz_assert(draw_indirect_buffer.get_target() == BufferTarget::DrawIndirect, "Passed non-draw-indirect buffer to VertexArray::draw_indexed_indirect(...). Please submit a bug report.");
		this->bind();
		index_buffer.basic_bind();
		draw_indirect_buffer.basic_bind();
		glMultiDrawElementsIndirect(tessellation ? GL_PATCHES : GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, draw_count, 0);
	}

	VertexArray VertexArray::null()
	{
		return {nullptr};
	}

	VertexArray::VertexArray(std::nullptr_t):
	vao(0){}
}

#endif // TZ_OGL
