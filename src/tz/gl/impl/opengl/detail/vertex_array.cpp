#if TZ_OGL
#include "tz/core/profile.hpp"
#include "tz/gl/impl/opengl/detail/vertex_array.hpp"
#include "tz/gl/impl/opengl/detail/buffer.hpp"

namespace tz::gl::ogl2
{
	vertex_array::vertex_array():
	vao(0)
	{
		TZ_PROFZONE("ogl - vao create", 0xFFAA0000);
		tz::assert(ogl2::is_initialised(), "Tried to create vertex array because ogl2 was not initialised. Please submit a bug report.");
		glCreateVertexArrays(1, &this->vao);
	}

	vertex_array::vertex_array(vertex_array&& move):
	vao(0)
	{
		*this = std::move(move);
	}

	vertex_array::~vertex_array()
	{
		TZ_PROFZONE("ogl - vao delete", 0xFFAA0000);
		glDeleteVertexArrays(1, &this->vao);
	}

	vertex_array& vertex_array::operator=(vertex_array&& rhs)
	{
		std::swap(this->vao, rhs.vao);
		return *this;
	}

	void vertex_array::bind()
	{
		TZ_PROFZONE("ogl - vao bind", 0xFFAA0000);
		glBindVertexArray(this->vao);
	}

	void vertex_array::draw(unsigned int primitive_count, primitive_topology topology, bool tessellation)
	{
		TZ_PROFZONE("ogl - draw", 0xFFAA0000);
		this->bind();
		glDrawArrays(tessellation ? GL_PATCHES : static_cast<GLenum>(topology), 0, primitive_count * primitive_vertex_count(topology));
	}

	void vertex_array::draw_indexed(unsigned int primitive_count, const buffer& index_buffer, primitive_topology topology, bool tessellation)
	{
		TZ_PROFZONE("ogl - draw indexed", 0xFFAA0000);
		tz::assert(!index_buffer.is_null(), "Detected `nullbuf` index buffer for draw_indexed");
		this->bind();
		index_buffer.custom_bind(buffer_target::index);
		glDrawElements(tessellation ? GL_PATCHES : static_cast<GLenum>(topology), primitive_count * primitive_vertex_count(topology), GL_UNSIGNED_INT, nullptr);
	}

	void vertex_array::draw_indirect(unsigned int draw_count, const buffer& draw_indirect_buffer, primitive_topology topology, bool tessellation)
	{
		TZ_PROFZONE("ogl - draw indirect", 0xFFAA0000);
		tz::assert(!draw_indirect_buffer.is_null(), "Detected `nullbuf` draw indirect buffer for draw indirect");
		this->bind();
		draw_indirect_buffer.custom_bind(buffer_target::draw_indirect);
		glMultiDrawArraysIndirect(tessellation ? GL_PATCHES : static_cast<GLenum>(topology), nullptr, draw_count, 0);
	}

	void vertex_array::draw_indirect_count(unsigned int max_draw_count, const buffer& draw_indirect_buffer, std::uintptr_t draw_commands_offset, primitive_topology topology, bool tessellation)
	{
		TZ_PROFZONE("ogl - draw indirect count", 0xFFAA0000);
		tz::assert(!draw_indirect_buffer.is_null(), "Detected `nullbuf` draw indirect buffer for draw indirect count");
		this->bind();
		draw_indirect_buffer.custom_bind(buffer_target::draw_indirect);
		draw_indirect_buffer.custom_bind(buffer_target::parameter);
		glMultiDrawArraysIndirectCountARB(tessellation ? GL_PATCHES : static_cast<GLenum>(topology), reinterpret_cast<const GLvoid*>(draw_commands_offset), 0, max_draw_count, 0);
	}

	void vertex_array::draw_indexed_indirect(unsigned int draw_count, const buffer& index_buffer, const buffer& draw_indirect_buffer, primitive_topology topology, bool tessellation)
	{
		TZ_PROFZONE("ogl - draw indexed indirect", 0xFFAA0000);
		tz::assert(!index_buffer.is_null(), "Detected `nullbuf` index buffer for draw indexed indirect");
		tz::assert(!draw_indirect_buffer.is_null(), "Detected `nullbuf` draw indirect buffer for draw indexed indirect");
		this->bind();
		index_buffer.custom_bind(buffer_target::index);
		draw_indirect_buffer.custom_bind(buffer_target::draw_indirect);
		glMultiDrawElementsIndirect(tessellation ? GL_PATCHES : static_cast<GLenum>(topology), GL_UNSIGNED_INT, nullptr, draw_count, 0);
	}

	void vertex_array::draw_indexed_indirect_count(unsigned int max_draw_count, const buffer& index_buffer, const buffer& draw_indirect_buffer, std::uintptr_t draw_commands_offset, primitive_topology topology, bool tessellation)
	{
		TZ_PROFZONE("ogl - draw indexed indirect count", 0xFFAA0000);
		tz::assert(!index_buffer.is_null(), "Detected `nullbuf` index buffer for draw indexed indirect count");
		tz::assert(!draw_indirect_buffer.is_null(), "Detected `nullbuf` draw indirect buffer for draw indexed indirect count");
		this->bind();
		index_buffer.custom_bind(buffer_target::index);
		draw_indirect_buffer.custom_bind(buffer_target::draw_indirect);
		draw_indirect_buffer.custom_bind(buffer_target::parameter);
		glMultiDrawElementsIndirectCountARB(tessellation ? GL_PATCHES : static_cast<GLenum>(topology), GL_UNSIGNED_INT, reinterpret_cast<const GLvoid*>(draw_commands_offset), 0, max_draw_count, 0);
	}

	vertex_array vertex_array::null()
	{
		return {nullptr};
	}

	vertex_array::vertex_array(std::nullptr_t):
	vao(0){}
}

#endif // TZ_OGL
