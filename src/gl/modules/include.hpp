#ifndef TOPAZ_GL_MODULES_INCLUDE_HPP
#define TOPAZ_GL_MODULES_INCLUDE_HPP
#include "gl/shader_preprocessor.hpp"

namespace tz::gl::p
{
	/**
	 * Handles #include preprocessor directives in arbitrary strings. Replaces the #include directive with the contents of the include file.
	 * 
	 * Note: This is similar to the #include directive in C++.
	 * Expects the include path to be relative to the parent-directory of include.cpp.
	 */
	class IncludeModule : public IModule
	{
	public:
		IncludeModule(std::string source_path);
		virtual void operator()(std::string& source) const override;
	private:
		std::string cat_include(std::string include_path) const;
		void register_replacement(std::size_t string_pos, std::size_t erasure_length, const std::string& replacement);

		std::string path;
	};
}

#endif // TOPAZ_GL_MODULES_INCLUDE_HPP