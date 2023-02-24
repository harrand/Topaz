#ifndef TOPAZ_GL_IMPL_BACKEND_OGL2_VERTEX_ARRAY_HPP
#define TOPAZ_GL_IMPL_BACKEND_OGL2_VERTEX_ARRAY_HPP
#if TZ_OGL
#include "tz/gl/impl/opengl/detail/tz_opengl.hpp"

namespace tz::gl::ogl2
{
	class Buffer;
	/**
	 * @ingroup tz_gl_ogl2
	 * Wrapper for an OpenGL VAO.
	 */
	class vertex_array
	{
	public:
		/**
		 * Create a new VAO.
		 */
		vertex_array();
		vertex_array(const vertex_array& copy) = delete;
		vertex_array(vertex_array&& move);
		~vertex_array();
		vertex_array& operator=(const vertex_array& rhs) = delete;
		vertex_array& operator=(vertex_array&& rhs);

		/**
		 * Bind the VAO, causing subsequent GL commands using VAO state to use this.
		 */
		void bind();
		/**
		 * Emit a single draw call, drawing a set number of triangles. Remember that vertex attributes are not supported in this backend, so you will source input data from either hard-coded shader values or from UBO/SSBO shader resources.
		 * @param triangle_count number of triangles to draw.
		 */
		void draw(unsigned int triangle_count, bool tessellation = false);
		/**
		 * Emit a single draw call, drawing a set number of triangles, assuming an index buffer has already been bound. Remember that vertex attributes are not supported in this backend, so you will source input data from either hard-coded shader values or from UBO/SSBO shader resources.
		 * @param triangle_count number of triangles to draw.
		 */
		void draw_indexed(unsigned int triangle_count, const Buffer& index_buffer, bool tessellation = false);

		void draw_indirect(unsigned int draw_count, const Buffer& draw_indirect_buffer, bool tessellation = false);
		void draw_indirect_count(unsigned int max_draw_count, const Buffer& draw_indirect_buffer, std::uintptr_t draw_commands_offset, bool tessellation = false);
		void draw_indexed_indirect(unsigned int draw_count, const Buffer& index_buffer, const Buffer& draw_indirect_buffer, bool tessellation = false);
		void draw_indexed_indirect_count(unsigned int max_draw_count, const Buffer& index_buffer, const Buffer& draw_indirect_buffer, std::uintptr_t draw_commands_offset, bool tessellation = false);
		/**
		 * Retrieve the Null vertex_array. Binding the null vertex array is equivalent to unbinding a vertex array. It is invalid to attempt to perform draws or computes using the null vertex array.
		 */
		static vertex_array null();
		/**
		 * Query as to whether this is a null vertex array, which is equivalent to @ref vertex_array::null().
		 */
		bool is_null() const;
	private:
		vertex_array(std::nullptr_t);

		GLuint vao;
	};
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_BACKEND_OGL2_VERTEX_ARRAY_HPP
