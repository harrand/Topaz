#include "graphics/shader.hpp"
#include "utility/string.hpp"
#include "io/file.hpp"
#include "mesh.hpp"
#include <regex>

#ifdef TOPAZ_OPENGL
namespace tz::gl
{
	namespace detail
	{
        IncludeTracker::IncludeTracker(): path_source_container(), path_includes(), current_source_path(std::nullopt){}

		void IncludeTracker::set_source(std::string path, std::string source, bool remember)
		{
            if(remember)
            {
                this->current_source_path = {path};
            }
			this->path_source_container[path] = tz::utility::string::split_string(source, '\n');
		}

        void IncludeTracker::clear_cache()
        {
            this->current_source_path = {std::nullopt};
        }

		void IncludeTracker::set_include(std::string path, std::string include, std::size_t line_number)
		{
			std::size_t source_line_count = this->path_source_container[path].size();
			topaz_assert(source_line_count >= line_number, "IncludeTracker::set_include(...): Attempted to set an include for the source file \"", path, "\" at line number ", line_number, ", but my records indicate that it is only ", source_line_count, " lines long!");
			this->path_includes[path].insert({line_number, include});
		}

        void IncludeTracker::set_include_cached(std::string include, std::size_t line_number)
        {
            topaz_assert(this->current_source_path.has_value(), "IncludeTracker::set_include_cached(...): Attempted to set include for a cached source file, but there is nothing currently in the path cache!");
            this->set_include(this->current_source_path.value(), include, line_number);
        }

		std::optional<std::pair<std::size_t, std::string>> IncludeTracker::get_real_source_location(std::string path, std::size_t line_number) const
		{
			// Say we have something like this:
			/*
			 * file1.glsl:
			 * 1| hello there
			 * 2| this is some code
			 * 3| #include "file2.glsl"
			 * 4| file2_foo();
			 *
			 * file2.glsl:
			 * 1| well met
			 * 2| void file2_foo():
			 * 3|     blah();
			 * 4| thanks for listening to my ted talk
			 *
			 * preprocessed file1.glsl:
			 * 1| hello there
			 * 2| this is some code
			 * 3| well met
			 * 4| void file2_foo():
			 * 5|     blah();
			 * 6| thanks for listening to my ted talk 		<---- error here!
			 * 7| file2_foo();
			 *
			 * Naively we would end up with an error at file1.glsl:6
			 * Which looks weird because file1.glsl only has 4 lines really!
			 * This method ideally would give us file1.glsl:4
			 */
			std::size_t cumulative_line_count = this->path_source_container.at(path).size();
			// set will be ordered by line number that includes it (i.e in the order we expect)
			const std::set<Include> includes = this->path_includes.at(path);
			for(const auto& [line_occurrence, include_path] : includes)
			{
				std::size_t count_post_include = cumulative_line_count + line_occurrence;
				if(count_post_include > line_number)
				{
					// this is the file which has the error!
					std::size_t line_difference = line_number - cumulative_line_count;
					return {std::pair<std::size_t, std::string>{line_difference, include_path}};
				}
			}
			return {std::nullopt};
		}
	}

	OGLShader::OGLShader(const std::string& vertex_source, const std::string& tessellation_control_source, const std::string& tessellation_evaluation_source, const std::string& geometry_source, const std::string& fragment_source, bool compile, bool link, bool validate, ShaderAttributeCollection attribute_collection): OGLShaderProgram(), attribute_collection(attribute_collection), include_tracker()
	{
		if(compile)
		{
			this->emplace_shader_component(OGLShaderComponentType::VERTEX, vertex_source).get_compile_result().report_if_fail(std::cout);
			if (tessellation_control_source != "")
				this->emplace_shader_component(OGLShaderComponentType::TESSELLATION_CONTROL, tessellation_control_source).get_compile_result().report_if_fail(std::cout);
			if (tessellation_evaluation_source != "")
				this->emplace_shader_component(OGLShaderComponentType::TESSELLATION_EVALUATION, tessellation_evaluation_source).get_compile_result().report_if_fail(std::cout);
			if (geometry_source != "")
				this->emplace_shader_component(OGLShaderComponentType::GEOMETRY, geometry_source).get_compile_result().report_if_fail(std::cout);
			this->emplace_shader_component(OGLShaderComponentType::FRAGMENT, fragment_source).get_compile_result().report_if_fail(std::cout);
		}
		if(compile && link)
		{
			this->setup_attributes();
			this->link().report_if_fail(std::cout);
		}
		if(compile && link && validate)
			this->validate().report_if_fail(std::cout);
	}

	OGLShader::OGLShader(std::string path, bool compile, bool link, bool validate, ShaderAttributeCollection attribute_collection): OGLShader(this->parse_source(path, ::tz::utility::file::read(path + ".vertex.glsl")), this->parse_source(path, ::tz::utility::file::read(path + ".tessellation_control.glsl")), this->parse_source(path, ::tz::utility::file::read(path + ".tessellation_evaluation.glsl")), this->parse_source(path, ::tz::utility::file::read(path + ".geometry.glsl")), this->parse_source(path, ::tz::utility::file::read(path + ".fragment.glsl")), compile, link, validate, attribute_collection)
    {
        // testing purposes only
        std::string prepreprocessed = tz::utility::file::read(path + ".fragment.glsl");
        std::string preprocessed = this->parse_source(path, prepreprocessed);
        tz::debug::print("before preprocessing: \n=============================\n", prepreprocessed, "\n=============================\nafter preprocessing: \n=============================\n", preprocessed, "\n=============================\n");
    }

	void OGLShader::setup_attributes() const
	{
		using namespace tz::gl;
		using namespace tz::consts::graphics::mesh;
		for(GLuint i = 0; i < this->attribute_collection.get_size(); i++)
		{
			this->bind_attribute_location(i, this->attribute_collection[i]);
		}
	}

	std::string OGLShader::parse_source(const std::string& path, const std::string& source) const
	{
        //this->include_tracker.set_source(path, source, true);
		return this->parse_static_prints(path, this->include_headers(path, source));
	}

	std::string OGLShader::include_headers(const std::string& path, const std::string& source) const
	{
		std::vector<std::string> source_lines = tz::utility::string::split_string(source, '\n');
		std::string parsed_source;
		for(std::size_t i = 0; i < source_lines.size(); i++)
		{
            std::string& line = source_lines[i];
			if(tz::utility::string::begins_with(tz::utility::string::stripped(line), "#include"))
			{
				// Need to handle include. Includes must be relative to the PROGRAM LOCATION.
				tz::debug::print("OGLShader::include_headers(string&): Detected header include:\n");
				std::string include_path = path.substr(0, path.find_last_of("/\\")) + "/" + tz::utility::string::substring(line, 10, line.size() - 2);
				tz::debug::print("\tInclude path (relative to directory containing this shader) = \"", include_path, "\"\n");
				std::string include_source = tz::utility::file::read(include_path);
                // TODO: Make note of the include source data for tracking aswell...
                //this->include_tracker.set_source(include_path, include_source);
                //this->include_tracker.set_include_cached(include_path, i);
				tz::debug::print("\tShader Source:\n", include_source);
				line = include_source;
			}
			parsed_source += line + "\n";
		}
		return parsed_source;
	}

	std::string OGLShader::parse_static_prints(const std::string& path, const std::string& source) const
	{
		std::vector<std::string> source_lines = tz::utility::string::split_string(source, '\n');
        std::string parsed_source;
        for(auto& line : source_lines)
		{
			if(tz::utility::string::begins_with(tz::utility::string::stripped(line), "#static_print"))
			{
				std::regex rgx{"#static_print\\(\"(.+)\"\\)"};
				std::smatch match;
                std::string line_stripped = tz::utility::string::stripped(line, true);
				std::regex_match(line_stripped, match, rgx);
				topaz_assert(match.size() > 1, "Static assertation malformed! Source line: \"", line, "\"");
                tz::debug::print("Found static print in shader path \"", path, "\" at line ", line, "\n");
                // Invoke the static print (seeing as this is compile time).
				std::cout << "Shader (" << path << ") -- Static Print:\t" << match[1] << "\n";
			}
            else
            {
                // Source falls through if it's not been preprocessed.
                parsed_source += line + "\n";
            }
		}
        // This is identical to the input source, but all lines containing a static-print are invoked and parsed out.
		return parsed_source;
	}
}
#endif
const char* tz::util::shader_type_string(GLenum shader_type)
{
	const char *shader_type_str;
	switch (shader_type)
	{
		case GL_COMPUTE_SHADER:
			shader_type_str = "Compute";
			break;
		case GL_VERTEX_SHADER:
			shader_type_str = "Vertex";
			break;
		case GL_TESS_CONTROL_SHADER:
			shader_type_str = "Tessellation Control";
			break;
		case GL_TESS_EVALUATION_SHADER:
			shader_type_str = "Tessellation Evaluation";
			break;
		case GL_GEOMETRY_SHADER:
			shader_type_str = "Geometry";
			break;
		case GL_FRAGMENT_SHADER:
			shader_type_str = "Fragment";
			break;
		default:
			shader_type_str = "Unknown";
			break;
	}
	return shader_type_str;
}