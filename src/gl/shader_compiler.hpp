#ifndef TOPAZ_GL_SHADER_COMPILER_HPP
#define TOPAZ_GL_SHADER_COMPILER_HPP
#include <string>

namespace tz::gl
{
	// Forward declares
	class Shader;
	class ShaderProgram;

	/**
	 * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
	 * @{
	 */

	enum class ShaderCompilerType
	{
		Auto // NYFI. Will be a ShaderCompiler fitting the current Topaz build configuration.
	};

	/**
	 * Options used for a given ShaderCompiler. This contains all the possible variables differing among various ShaderCompilers.
	 * Note: At present, there is only one ShaderCompiler anyway...
	 */
	struct ShaderCompilerOptions
	{
		ShaderCompilerType type;
	};

	namespace detail
	{
		constexpr ShaderCompilerOptions get_default_compiler_options()
		{
			return {ShaderCompilerType::Auto};
		}
	}

	/**
	 * Diagnostics are the result from compilation and link attempts by a ShaderCompiler.
	 * 
	 * In short, diagnostics will know the result of the process (success/fail) and some additional logging information from the graphics driver if the process failed.
	 */
	class ShaderCompilerDiagnostic
	{
	public:
		/**
		 * Construct a diagnostic from a previous process.
		 * @param success Whether the process was successful or not.
		 * @param info_log Additional information on failure. This should be empty if the process was successful.
		 */
		ShaderCompilerDiagnostic(bool success, std::string info_log);
		/**
		 * Query as to whether this diagnostic indicated success.
		 * @return True if successful, false on failure.
		 */
		bool successful() const;
		/**
		 * Retrieve additional information about the process runtime. This will be empty if the process was successful.
		 */
		const std::string& get_info_log() const;
	private:
		bool success;
		std::string info_log;
	};

	/**
	 * ShaderCompilers are used to create programs which are designed to run on the GPU.
	 * 
	 * All ShaderCompilers have two responsibilities:
	 * - Compile Shader components using graphics drivers.
	 * - Link ShaderPrograms to produce a valid executable for the GPU.
	 */
	class ShaderCompiler
	{
	public:
		/**
		 * Construct a compiler using the given options.
		 * 
		 * The compiler is driven by the OpenGL drivers installed on the system. The configuration of this is done in tz::initialise.
		 * @param options Compiler options for this compiler.
		 */
		ShaderCompiler(ShaderCompilerOptions options = detail::get_default_compiler_options());
		/**
		 * Attempt to compile the given Shader component.
		 * Precondition: The Shader component should have previously had valid source-code uploaded to it. Otherwise, this will assert and will return a failure diagnostic.
		 * @param shader Shader whose uploaded source should be compiled.
		 * @return Diagnostic information from the attempted compilation.
		 */
		ShaderCompilerDiagnostic compile(Shader& shader) const;
		/**
		 * Attempt to link the given ShaderProgram.
		 * @param program ShaderProgram to link component data to produce an executable for the GPU.
		 * @return Diagnostic information from the attempted linkage and validation.
		 */
		ShaderCompilerDiagnostic link(ShaderProgram& program) const;
	private:
		ShaderCompilerOptions options;
	};

	/**
	 * @}
	 */
}

#endif // TOPAZ_GL_SHADER_COMPILER_HPP