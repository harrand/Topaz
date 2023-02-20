#if TZ_OGL
#include "tz/core/profile.hpp"
#include "tz/gl/impl/opengl/detail/vertex_array.hpp"
#include "tz/gl/impl/opengl/detail/buffer.hpp"

namespace tz::gl::ogl2
{
	VertexArray::VertexArray():
	vao(0)
	{
		TZ_PROFZONE("OpenGL Backend - VertexArray Create", 0xFFAA0000);
		tz::assert(ogl2::is_initialised(), "Tried to create vertex array because ogl2 was not initialised. Please submit a bug report.");
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
		TZ_PROFZONE("OpenGL Backend - VertexArray Bind", 0xFFAA0000);
		glBindVertexArray(this->vao);
	}

	void VertexArray::draw(unsigned int triangle_count, bool tessellation)
	{
		TZ_PROFZONE("OpenGL Backend - VertexArray Draw", 0xFFAA0000);
		this->bind();
		glDrawArrays(tessellation ? GL_PATCHES : GL_TRIANGLES, 0, triangle_count * 3);
	}

	void VertexArray::draw_indexed(unsigned int triangle_count, const Buffer& index_buffer, bool tessellation)
	{
		TZ_PROFZONE("OpenGL Backend - VertexArray DrawIndexed", 0xFFAA0000);
		//tz::assert(index_buffer.get_target() == BufferTarget::Index, "Passed non-index buffer to VertexArray::draw_indexed(...). Please submit a bug report.");
		// Note: Normally basic bind (glBindBuffer) has nothing to do with the VAO. Index buffers (GL_ELEMENT_ARRAY_BUFFER) bindings are an exception - this is explicitly associated with the currently-bound VAO. That is why we need to have the index buffer passed in via param so we can bind the vao before actually trying to bind the buffer.
		this->bind();
		index_buffer.custom_bind(BufferTarget::Index);
		glDrawElements(tessellation ? GL_PATCHES : GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_INT, nullptr);
	}

	void VertexArray::draw_indirect(unsigned int draw_count, const Buffer& draw_indirect_buffer, bool tessellation)
	{
		TZ_PROFZONE("OpenGL Backend - VertexArray DrawIndirect", 0xFFAA0000);
		//tz::assert(draw_indirect_buffer.get_target() == BufferTarget::DrawIndirect, "Passed non-draw-indirect buffer to VertexArray::draw_indirect(...). Please submit a bug report.");
		this->bind();
		draw_indirect_buffer.custom_bind(BufferTarget::DrawIndirect);
		glMultiDrawArraysIndirect(tessellation ? GL_PATCHES : GL_TRIANGLES, nullptr, draw_count, 0);
	}

	void VertexArray::draw_indirect_count(unsigned int max_draw_count, const Buffer& draw_indirect_buffer, std::uintptr_t draw_commands_offset, bool tessellation)
	{
		TZ_PROFZONE("OpenGL Backend - VertexArray DrawIndirectCount", 0xFFAA0000);
		this->bind();
		draw_indirect_buffer.custom_bind(BufferTarget::DrawIndirect);
		draw_indirect_buffer.custom_bind(BufferTarget::Parameter);
		glMultiDrawArraysIndirectCountARB(tessellation ? GL_PATCHES : GL_TRIANGLES, reinterpret_cast<const GLvoid*>(draw_commands_offset), 0, max_draw_count, 0);
	}

	void VertexArray::draw_indexed_indirect(unsigned int draw_count, const Buffer& index_buffer, const Buffer& draw_indirect_buffer, bool tessellation)
	{
		TZ_PROFZONE("OpenGL Backend - VertexArray DrawIndexedIndirect", 0xFFAA0000);
		//tz::assert(index_buffer.get_target() == BufferTarget::Index, "Passed non-index buffer to VertexArray::draw_indexed_indirect(...). Please submit a bug report.");
		//tz::assert(draw_indirect_buffer.get_target() == BufferTarget::DrawIndirect, "Passed non-draw-indirect buffer to VertexArray::draw_indexed_indirect(...). Please submit a bug report.");
		this->bind();
		index_buffer.custom_bind(BufferTarget::Index);
		draw_indirect_buffer.custom_bind(BufferTarget::DrawIndirect);
		glMultiDrawElementsIndirect(tessellation ? GL_PATCHES : GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, draw_count, 0);
	}

	void VertexArray::draw_indexed_indirect_count(unsigned int max_draw_count, const Buffer& index_buffer, const Buffer& draw_indirect_buffer, std::uintptr_t draw_commands_offset, bool tessellation)
	{
		TZ_PROFZONE("OpenGL Backend - VertexArray DrawIndexedIndirectCount", 0xFFAA0000);
		this->bind();
		index_buffer.custom_bind(BufferTarget::Index);
		draw_indirect_buffer.custom_bind(BufferTarget::DrawIndirect);
		draw_indirect_buffer.custom_bind(BufferTarget::Parameter);
		glMultiDrawElementsIndirectCountARB(tessellation ? GL_PATCHES : GL_TRIANGLES, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid*>(draw_commands_offset), 0, max_draw_count, 0);
	}

	VertexArray VertexArray::null()
	{
		return {nullptr};
	}

	VertexArray::VertexArray(std::nullptr_t):
	vao(0){}
}

#endif // TZ_OGL
