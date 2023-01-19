#ifndef TOPAZ_GL_IMPL_BACKEND_OGL2_SHADER_HPP
#define TOPAZ_GL_IMPL_BACKEND_OGL2_SHADER_HPP
#if TZ_OGL
#include "tz/core/containers/basic_list.hpp"
#include "tz/gl/impl/opengl/detail/tz_opengl.hpp"

namespace tz::gl::ogl2
{
	/**
	 * @ingroup tz_gl_ogl2_shader
	 * Specifies the shader type.
	 */
	enum class ShaderType : GLenum
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
	 * Specifies creation flags for a @ref ShaderModule.
	 */
	struct ShaderModuleInfo
	{
		/// Type of this shader module.
		ShaderType type;
		/// GLSL source code.
		OGLString code;
	};
	
	/**
	 * @ingroup tz_gl_ogl2_shader
	 * Specifies creation flags for a @ref Shader.
	 */
	struct ShaderInfo
	{
		/// List of all modules.
		tz::BasicList<ShaderModuleInfo> modules;
	};

	/**
	 * @ingroup tz_gl_ogl2_shader
	 * Represents an OpenGL shader.
	 */
	class ShaderModule
	{
	public:
		/**
		 * State of a shader module compilation attempt.
		 */
		struct CompileResult
		{
			/// True if compilation was successful, otherwise false.
			bool success;
			/// String containing information about compilation. If compilation was successful, this is guaranteed to be the empty string.
			OGLString info_log;

			operator bool() const {return this->success;}
		};

		/**
		 * Create a new shader module. You should not need to invoke this directly - The shader program is responsible for constructing all of its modules.
		 */
		ShaderModule(ShaderModuleInfo info);
		ShaderModule(const ShaderModule& copy) = delete;
		ShaderModule(ShaderModule&& move);
		~ShaderModule();
		ShaderModule& operator=(const ShaderModule& rhs) = delete;
		ShaderModule& operator=(ShaderModule&& rhs);

		ShaderType get_type() const;

		/**
		 * Attempt to compile the shader source. This could fail.
		 * @return Result state describing the success of the compilation attempt.
		 */
		CompileResult compile();

		using NativeType = GLuint;
		NativeType native() const;
	private:
		GLuint shader;
		ShaderModuleInfo info;
	};

	/**
	 * @ingroup tz_gl_ogl2_shader
	 * Represents an OpenGL shader program.
	 */
	class Shader
	{
	public:
		/**
		 * State of a shader module compilation attempt.
		 */
		struct LinkResult
		{
			/// True if linkage + validation were successful, otherwise false.
			bool success;
			/// String containing information about compilation. If compilation was successful, this is guaranteed to be the empty string.
			OGLString info_log;

			operator bool() const {return this->success;}
		};
		/**
		 * Create a new shader program.
		 * @param info Information about modules contained within the shader program.
		 */
		Shader(ShaderInfo info);
		Shader(const Shader& copy) = delete;
		Shader(Shader&& move);
		~Shader();
		Shader& operator=(const Shader& rhs) = delete;
		Shader& operator=(Shader&& rhs);

		/**
		 * Attempt to link and validate the shader program. This could fail.
		 * @return Result state describing the success of the link + validate attempts.
		 */
		LinkResult link();
		/**
		 * Set the program as in-use, causing subsequent gl commands to use it as the shader program.
		 */
		void use() const;

		/**
		 * Create the null shader. Operations are invalid on the null shader.
		 */
		static Shader null();
		/**
		 * Query as to whether this is a null shader. See @ref Shader::null().
		 */
		bool is_null() const;

		bool is_compute() const;
		bool has_tessellation() const;

		std::string debug_get_name() const;
		void debug_set_name(std::string name);
	private:
		Shader(std::nullptr_t);
		Shader::LinkResult validate();

		GLuint program;
		std::vector<ShaderModule> modules;
		ShaderInfo info;
		std::string debug_name = "";
	};
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_BACKEND_OGL2_SHADER_HPP
