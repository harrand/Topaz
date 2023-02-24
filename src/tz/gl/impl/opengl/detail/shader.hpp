#ifndef TOPAZ_GL_IMPL_BACKEND_OGL2_SHADER_HPP
#define TOPAZ_GL_IMPL_BACKEND_OGL2_SHADER_HPP
#if TZ_OGL
#include "tz/core/data/basic_list.hpp"
#include "tz/gl/impl/opengl/detail/tz_opengl.hpp"

namespace tz::gl::ogl2
{
	/**
	 * @ingroup tz_gl_ogl2_shader
	 * Specifies the shader type.
	 */
	enum class shader_type : GLenum
	{
		/// - Vertex Shader.
		vertex = GL_VERTEX_SHADER,
		/// - Tessellation Control Shader.
		tessellation_control = GL_TESS_CONTROL_SHADER,
		/// - Tessellation Evaluation Shader.
		tessellation_evaluation = GL_TESS_EVALUATION_SHADER,
		/// - Fragment Shader.
		fragment = GL_FRAGMENT_SHADER,
		/// - Compute Shader.
		compute = GL_COMPUTE_SHADER
	};

	/**
	 * @ingroup tz_gl_ogl2_shader
	 * Specifies creation flags for a @ref shader_module.
	 */
	struct shader_module_info
	{
		/// Type of this shader module.
		shader_type type;
		/// GLSL source code.
		ogl_string code;
	};
	
	/**
	 * @ingroup tz_gl_ogl2_shader
	 * Specifies creation flags for a @ref Shader.
	 */
	struct shader_info
	{
		/// List of all modules.
		tz::basic_list<shader_module_info> modules;
	};

	/**
	 * @ingroup tz_gl_ogl2_shader
	 * Represents an OpenGL shader.
	 */
	class shader_module
	{
	public:
		/**
		 * State of a shader module compilation attempt.
		 */
		struct compile_result
		{
			/// True if compilation was successful, otherwise false.
			bool success;
			/// String containing information about compilation. If compilation was successful, this is guaranteed to be the empty string.
			ogl_string info_log;

			operator bool() const {return this->success;}
		};

		/**
		 * Create a new shader module. You should not need to invoke this directly - The shader program is responsible for constructing all of its modules.
		 */
		shader_module(shader_module_info info);
		shader_module(const shader_module& copy) = delete;
		shader_module(shader_module&& move);
		~shader_module();
		shader_module& operator=(const shader_module& rhs) = delete;
		shader_module& operator=(shader_module&& rhs);

		shader_type get_type() const;

		/**
		 * Attempt to compile the shader source. This could fail.
		 * @return Result state describing the success of the compilation attempt.
		 */
		compile_result compile();

		using NativeType = GLuint;
		NativeType native() const;
	private:
		GLuint shader;
		shader_module_info info;
	};

	/**
	 * @ingroup tz_gl_ogl2_shader
	 * Represents an OpenGL shader program.
	 */
	class shader
	{
	public:
		/**
		 * State of a shader module compilation attempt.
		 */
		struct link_result
		{
			/// True if linkage + validation were successful, otherwise false.
			bool success;
			/// String containing information about compilation. If compilation was successful, this is guaranteed to be the empty string.
			ogl_string info_log;

			operator bool() const {return this->success;}
		};
		/**
		 * Create a new shader program.
		 * @param info Information about modules contained within the shader program.
		 */
		shader(shader_info info);
		shader(const shader& copy) = delete;
		shader(shader&& move);
		~shader();
		shader& operator=(const shader& rhs) = delete;
		shader& operator=(shader&& rhs);

		/**
		 * Attempt to link and validate the shader program. This could fail.
		 * @return Result state describing the success of the link + validate attempts.
		 */
		link_result link();
		/**
		 * Set the program as in-use, causing subsequent gl commands to use it as the shader program.
		 */
		void use() const;

		/**
		 * Create the null shader. Operations are invalid on the null shader.
		 */
		static shader null();
		/**
		 * Query as to whether this is a null shader. See @ref Shader::null().
		 */
		bool is_null() const;

		bool is_compute() const;
		bool has_tessellation() const;

		std::string debug_get_name() const;
		void debug_set_name(std::string name);
	private:
		shader(std::nullptr_t);
		shader::link_result validate();

		GLuint program;
		std::vector<shader_module> modules;
		shader_info info;
		std::string debug_name = "";
	};
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_BACKEND_OGL2_SHADER_HPP
