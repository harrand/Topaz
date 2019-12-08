#ifndef SHADER_HPP
#define SHADER_HPP
#include "platform/ogl/shader_program.hpp"
#include "graphics/attribute_collection.hpp"
#include "data/tree.hpp"
#include <set>

#ifdef TOPAZ_OPENGL
namespace tz::gl
{
    namespace detail
    {
        class IncludeTracker
        {
        public:
            IncludeTracker();
            void set_source(std::string path, std::string source, bool remember = false);
			void clear_cache();
            void set_include(std::string path, std::string include, std::size_t line_number);
			void set_include_cached(std::string include, std::size_t line_number);
            std::optional<std::pair<std::size_t, std::string>> get_real_source_location(std::string path, std::size_t line_number) const;
        private:
            /// Line number and path to include file
            using Include = std::pair<std::size_t, std::string>;
            /// Maps a path to a list of source lines.
            std::unordered_map<std::string, std::vector<std::string>> path_source_container;
            /// Maps a path to a list of all paths that it includes (and the line that the source includes it).
            std::unordered_map<std::string, std::set<Include>> path_includes;
			/// Stores the current source-path if there is one. This makes setting includes easier.
			std::optional<std::string> current_source_path;
        };
    }

	class OGLShader : public OGLShaderProgram
	{
	public:
		OGLShader(const std::string& vertex_source, const std::string& tessellation_control_source, const std::string& tessellation_evaluation_source, const std::string& geometry_source, const std::string& fragment_source, bool compile = true, bool link = true, bool validate = true, ShaderAttributeCollection attribute_collection = {});
		OGLShader(std::string path, bool compile = true, bool link = true, bool validate = true, ShaderAttributeCollection attribute_collection = {});
		void setup_attributes() const;
	private:
        std::string parse_source(const std::string& path, const std::string& source) const;
		std::string include_headers(const std::string& path, const std::string& source) const;
		std::string parse_static_prints(const std::string& path, const std::string& source) const;

		ShaderAttributeCollection attribute_collection;
        // TODO: Use properly for accurate compile error source locations.
		mutable detail::IncludeTracker include_tracker;
	};
}
using Shader = tz::gl::OGLShader;
#endif

namespace tz
{
	namespace util
	{
		/**
		* Get a label for a shader_type (e.g GL_FRAGMENT_SHADER returns "Fragment")
		*/
		const char* shader_type_string(GLenum shader_type);
	}
}
#endif