#ifndef TOPAZ_GL_MODULES_INCLUDE_HPP
#define TOPAZ_GL_MODULES_INCLUDE_HPP
#include "gl/shader_preprocessor.hpp"

namespace tz::gl::p
{
	/**
     * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
     * @{
     */
	/**
     * \addtogroup tz_gl_p tz::gl Shader Preprocessing Module (TZGLP)
     * @{
     */

	/**
	 * Handles #include preprocessor directives in arbitrary strings. Replaces the #include directive with the contents of the include file.
	 * 
	 * Note: This is similar to the #include directive in C++.
	 * Expects the include path to be relative to the parent-directory of include.cpp.
	 */
	class IncludeModule : public IModule
	{
	public:
		/**
         * Construct an IncludeModule, searching for include-files relative to the given source-path.
		 * 
		 * Note: Include files in #include directives should be relative to the given source path.
         * @param source_path Parent directory path through which to search for include files.
         */
		IncludeModule(std::string source_path);
		/**
         * Invoke the module, performing source transformation on the given string by replacing all #include directives with the source code of the included file, if it could be found.
         * 
         * Note: Expect the stored SSBOs to be in-order as they appear when the source-code is read top-to-bottom.
		 * Precondition: All #include files in the source must be relative to the source path in the constructor. Otherwise, this will assert and fail to process includes correctly.
         * @param source GLSL source-code to preprocess. Note that it doesn't have to be valid code.
         */
		virtual void operator()(std::string& source) const override;
	private:
		std::string cat_include(std::string include_path) const;

		std::string path;
	};

	/**
	 * @}
	 */
	/**
	 * @}
	 */
}

#endif // TOPAZ_GL_MODULES_INCLUDE_HPP