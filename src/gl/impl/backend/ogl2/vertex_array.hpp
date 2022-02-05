#ifndef TOPAZ_GL_IMPL_BACKEND_OGL2_VERTEX_ARRAY_HPP
#define TOPAZ_GL_IMPL_BACKEND_OGL2_VERTEX_ARRAY_HPP
#if TZ_OGL
#include "gl/impl/backend/ogl2/tz_opengl.hpp"

namespace tz::gl::ogl2
{
	/**
	 * @ingroup tz_gl_ogl2
	 * Wrapper for an OpenGL VAO.
	 */
	class VertexArray
	{
	public:
		/**
		 * Create a new VAO.
		 */
		VertexArray();
		VertexArray(const VertexArray& copy) = delete;
		VertexArray(VertexArray&& move);
		~VertexArray();
		VertexArray& operator=(const VertexArray& rhs) = delete;
		VertexArray& operator=(VertexArray&& rhs);

		/**
		 * Bind the VAO, causing subsequent GL commands using VAO state to use this.
		 */
		void bind();

		/**
		 * Retrieve the Null VertexArray. Binding the null vertex array is equivalent to unbinding a vertex array. It is invalid to attempt to perform draws or computes using the null vertex array.
		 */
		static VertexArray null();
		/**
		 * Query as to whether this is a null vertex array, which is equivalent to @ref VertexArray::null().
		 */
		bool is_null() const;
	private:
		VertexArray(std::nullptr_t);

		GLuint vao;
	};
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_BACKEND_OGL2_VERTEX_ARRAY_HPP
