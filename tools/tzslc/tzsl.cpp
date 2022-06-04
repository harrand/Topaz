#include "tzsl.hpp"
#include "source_transform.hpp"
#include <fstream>

#include "stdlib.hpp"

namespace tzslc
{
	enum class ShaderStage
	{
		Compute,
		Vertex,
		Fragment,
		Count
	};

	std::string default_defines(GLSLDialect dialect);
	ShaderStage try_get_stage(const std::string&);
	void evaluate_imports(std::string&);
	void evaluate_user_imports(std::string&, std::filesystem::path);
	void evaluate_keywords(std::string&, ShaderStage, GLSLDialect);
	void evaluate_inout_blocks(std::string&, ShaderStage, GLSLDialect);
	void evaluate_language_level_functions(std::string&, GLSLDialect);
	void collapse_namespaces(std::string&);

	void rename_user_main(std::string& shader_source);
	void add_main_definition(std::string& shader_source);

//--------------------------------------------------------------------------------------------------

	void compile_to_glsl(std::string& shader_source, std::filesystem::path shader_filename, GLSLDialect dialect)
	{
		shader_source = default_defines(dialect) + shader_source;
		evaluate_imports(shader_source);
		evaluate_user_imports(shader_source, shader_filename);
		ShaderStage stage = try_get_stage(shader_source);
		tzslc_assert(stage != ShaderStage::Count, "Detected invalid shader stage. Internal tzslc error. Please submit a bug report.");
		evaluate_keywords(shader_source, stage, dialect);
		evaluate_inout_blocks(shader_source, stage, dialect);
		evaluate_language_level_functions(shader_source, dialect);
		collapse_namespaces(shader_source);

		if(stage == ShaderStage::Vertex)
		{
			rename_user_main(shader_source);
			add_main_definition(shader_source);
		}
	}

//--------------------------------------------------------------------------------------------------

	std::string default_defines(GLSLDialect dialect)
	{
		/*
		 * tzslc header info
		 * #version 450 core
		 * #define TZ_OGL ?
		 * #define TZ_VULKAN ?
		 * #define TZ_DEBUG ?
		 *
		 * 5 lines total
		 */
		std::string ret = "/*tzslc header info*/\n#version 450 core\n";
		if(dialect == GLSLDialect::Vulkan)
		{
			ret += "#define TZ_OGL 0\n#define TZ_VULKAN 1\n#extension GL_EXT_debug_printf : enable\n#extension GL_EXT_nonuniform_qualifier : enable\n";
		}
		else if(dialect == GLSLDialect::OpenGL)
		{
			ret += "#define TZ_OGL 1\n#define TZ_VULKAN 0\n#extension GL_ARB_bindless_texture : enable\n";
		}
		else
		{
			tzslc_error("ICE: Unrecognised GLSLDialect.");
		}
		return ret;
	}

//--------------------------------------------------------------------------------------------------

	void evaluate_imports(std::string& shader_source)
	{
		constexpr char import_regex[] = "import <([a-zA-Z0-9\\s]+)>";
		// Standard library imports.
		tzslc::transform(shader_source, std::regex{import_regex},
		[](auto beg, auto end)
		{
			const std::string& m = *beg;
			if(m == "atomic")
			{
				return std::string(stdlib_atomic);
			}
			if(m == "space")
			{
				return std::string(stdlib_space);
			}
			if(m == "math")
			{
				return std::string(stdlib_math);
			}
			if(m == "matrix")
			{
				return std::string(stdlib_matrix);
			}
			tzslc_error("Unknown stdlib import <%s>.", m.c_str());
			return std::string{""};
		});
	}


//--------------------------------------------------------------------------------------------------

	void evaluate_user_imports(std::string& shader_source, std::filesystem::path shader_filename)
	{
		constexpr char user_import_regex[] = "import \"([a-zA-Z0-9\\.\\s]+)\"";
		// User-defined imports
		tzslc::transform(shader_source, std::regex{user_import_regex},
		[shader_filename](auto beg, auto end)
		{
			const std::string& filename = *beg;
			std::filesystem::path full_path = shader_filename.parent_path() / (filename + ".tzsl");
			tzslc_assert(std::filesystem::exists(full_path), "import \"%s\" - Cannot find %s.tzsl.\nInclude Directory: %s", filename.c_str(), filename.c_str(), shader_filename.parent_path().string().c_str());
			std::ifstream import_file{full_path.c_str(), std::ios::ate | std::ios::binary};
			tzslc_assert(import_file.is_open(), "import \"%s\" - Shader file located no filesystem, but could not read for some reason. Read access denied?", filename.c_str());
			std::string buffer;
			
			buffer.resize(static_cast<std::size_t>(import_file.tellg()));
			import_file.seekg(0);
			import_file.read(buffer.data(), buffer.size());
			import_file.close();

			// The included file might have includes aswell, process them.
			evaluate_imports(buffer);
			evaluate_user_imports(buffer, full_path);
			return buffer;
		});
	}

//--------------------------------------------------------------------------------------------------

	ShaderStage try_get_stage(const std::string& src)
	{
		std::smatch match;
		bool found_specifier = std::regex_search(src, match, std::regex{"shader\\(type ?= ?([a-zA-Z]+)\\) ?;"});
		tzslc_assert(found_specifier, "Could not find shader stage specifier. TZSL source requires:\n`shader(type = ?)`\n where `?` is one of `vertex`, `fragment`, or `compute`");
		tzslc_assert(match.size() == 2, "Detected multiple shader stage specifiers. Exactly one shader stage specifier should exist in a compiled TZSL shader.");
		const std::string& specifier = match[1];
		if(specifier == "vertex")
		{
			return ShaderStage::Vertex;
		}
		else if(specifier == "fragment")
		{
			return ShaderStage::Fragment;
		}
		else if(specifier == "compute")
		{
			return ShaderStage::Compute;
		}
		else
		{
			tzslc_error("Unrecognised shader stage name `%s`. Expected either `vertex`, `fragment`, or `compute`", specifier.c_str());
			return ShaderStage::Count;
		}
	}

//--------------------------------------------------------------------------------------------------

	void evaluate_keywords(std::string& shader_source, ShaderStage stage, GLSLDialect dialect)
	{
		// Firstly, we'll evaluate stage specifiers.
		constexpr char shader_specifier_regex[] = "shader\\(type ?= ?([a-zA-Z]+)\\) ?;";

		std::size_t stage_specifier_count = 0;
		tzslc::transform(shader_source, std::regex{shader_specifier_regex},
		[&stage_specifier_count](auto beg, auto end)
		{
			stage_specifier_count++;
			return "#pragma shader_stage(" + *beg + ")";
		});

		tzslc_assert(stage_specifier_count == 1, "Unexpected number of shader stage specifiers. Expected 1, got %zu", stage_specifier_count);

		// Secondly, kernel specifiers for compute shaders.
		constexpr char kernel_regex[] = "kernel\\(([0-9]+), ?([0-9]+), ?([0-9]+)\\) ?;";

		bool kernel_specified = false;
		tzslc::transform(shader_source, std::regex{kernel_regex},
		[stage, &kernel_specified](auto beg, auto end)->std::string
		{
			kernel_specified = true;
			tzslc_assert(stage == ShaderStage::Compute, "Detected `kernel` specifier, but shader is not a compute shader. Kernel specifiers are only valid within compute shaders.");
			return "layout(local_size_x = " + *(beg) + ", local_size_y = " + *(beg + 1) + ", local_size_z = " + *(beg + 2) + ") in;";
		});

		tzslc_assert(kernel_specified == (stage == ShaderStage::Compute), "Missing `kernel` specifier for compute shader. A compute shader must specify a kernel exactly once.");

		// Thirdly, inputs.
		constexpr char input_regex[] = "input\\(id ?= ?([0-9]+)\\) (.*)";
		constexpr char flagged_input_regex[] = "input\\(id ?= ?([0-9]+), ?([a-zA-Z]+)\\) (.*)"; // Support for flat, etc...
		
		tzslc::transform(shader_source, std::regex{input_regex},
		[](auto beg, auto end)
		{
			return "layout(location = " + *(beg) + ") in " + *(beg + 1);
		});

		tzslc::transform(shader_source, std::regex{flagged_input_regex},
		[](auto beg, auto end)
		{
			const std::string& extra_specifier = *(beg + 1);
			tzslc_assert(extra_specifier == "flat", "`input(id = x, flag)`: Unrecognised flag \"%s\". Must be `flat`.", extra_specifier.c_str());
			return "layout(location = " + *(beg) + ") " + extra_specifier + " in " + *(beg + 2);
		});

		// After that, outputs.
		constexpr char output_regex[] = "output\\(id ?= ?([0-9]+)\\) (.*)";
		
		tzslc::transform(shader_source, std::regex{output_regex},
		[](auto beg, auto end)
		{
			return "layout(location = " + *(beg) + ") out " + *(beg + 1);
		});

		// Finally, resources.
		// Start with buffer resources.
		constexpr char buffer_resource_regex[] = "resource\\(id ?= ?([0-9]+)\\) ?([a-zA-Z]*) ?buffer";
		tzslc::transform(shader_source, std::regex{buffer_resource_regex},
		[](auto beg, auto end)
		{
			std::string flag = *(beg + 1);
			tzslc_assert(flag.empty() || (flag == "const"), "Detected unrecognised token `%s` in buffer resource specifier. Replace with nothing, or `const`.", flag.c_str());
			if(flag == "const")
			{
				flag = "readonly";
			}
			return "layout(binding = " + *(beg) + ") " + flag + " buffer";
		});

		// And then texture resources.
		constexpr char texture_resource_regex_vk[] = "resource\\(id ?= ?([0-9]+)\\) ?([a-zA-Z]*) ?texture";
		constexpr char texture_resource_regex_ogl[] = "resource\\(id ?= ?([0-9]+)\\) ?([a-zA-Z]*) ?texture (.+)\\[([0-9]*)\\]?;";
		// For VK, this is pretty easy.
		if(dialect == GLSLDialect::Vulkan)
		{
			tzslc::transform(shader_source, std::regex{texture_resource_regex_vk},
			[](auto beg, auto end)
			{
				std::string flag = *(beg + 1);
				tzslc_assert(flag == "const", "Detected unrecognised token `%s` in buffer resource specifier. Replace with nothing, or `const`. Non-const texture resources are not yet implemented.", flag.c_str());
				return "layout(binding = " + *(beg) + ") uniform sampler2D";
			});
		}
		else if(dialect == GLSLDialect::OpenGL)
		{
			tzslc::transform(shader_source, std::regex{texture_resource_regex_ogl},
			[](auto beg, auto end)
			{
				std::string flag = *(beg + 1);
				tzslc_assert(flag == "const", "Detected unrecognised token `%s` in buffer resource specifier. Replace with nothing, or `const`. Non-const texture resources are not yet implemented.", flag.c_str());
				return "layout(binding = " + *(beg) + ") buffer ImageData\n{\n\tsampler2D textures[" + *(beg + 3) + "];\n} " + *(beg + 2) + ";\n#define " + *(beg + 2) + " " + *(beg + 2) + ".textures";
			});
		}
		else
		{
			tzslc_error("ICE: Unrecognised GLSLDialect");
		}
	}

//--------------------------------------------------------------------------------------------------
	
	void evaluate_inout_blocks(std::string& shader_source, ShaderStage stage, GLSLDialect dialect)
	{
		auto xmog = [&shader_source](const char* from, const char* to)
		{
			shader_source = std::regex_replace(shader_source, std::regex{from}, to);
		};

		switch(stage)
		{
			case ShaderStage::Vertex:
			{
				const char* vertexid = nullptr;
				const char* instanceid = nullptr;
				if(dialect == GLSLDialect::Vulkan)
				{
					vertexid = "gl_VertexIndex";
					instanceid = "gl_InstanceIndex";
				}
				else if(dialect == GLSLDialect::OpenGL)
				{
					vertexid = "gl_VertexID";
					instanceid = "gl_InstanceID";
				}
				else
				{
					tzslc_error("ICE: Unrecognised GLSLDialect.");
				}
				xmog("in::vertex_id", vertexid);
				xmog("in::instance_id", instanceid);
				xmog("out::position", "gl_Position");
			}
			break;
			case ShaderStage::Fragment:
				xmog("in::fragment_coord", "gl_FragCoord");
				xmog("out::fragment_depth", "gl_FragDepth");
			break;
			case ShaderStage::Compute:
				xmog("in::workgroup_count", "gl_NumWorkGroups");
				xmog("in::workgroup_id", "gl_WorkGroupID");
				xmog("in::local_id", "gl_LocalInvocationID");
				xmog("in::global_id", "gl_GlobalInvocationID");
			break;
		}
	}

//--------------------------------------------------------------------------------------------------

	void evaluate_language_level_functions(std::string& shader_source, GLSLDialect dialect)
	{
		constexpr char tz_printf_regex[] = "tz_printf\\((.*)\\).*;";

		tzslc::transform(shader_source, std::regex{tz_printf_regex},
		[dialect](auto beg, auto end)
		{
			#if !NDEBUG
				if(dialect == GLSLDialect::Vulkan)
				{
					return "debugPrintfEXT(" + *(beg) + ");";
				}
				else
				{
					return std::string("");
				}
			#else
				return std::string("");
			#endif
		});
	}

//--------------------------------------------------------------------------------------------------

	void collapse_namespaces(std::string& shader_source)
	{
		// tz namespace gets special prefix. Other namespaces get just underscores.
		tzslc::transform(shader_source, std::regex{"tz::"},
		[](auto beg, auto end)
		{
			return "z_tz_stdlib_";
		});

		shader_source = std::regex_replace(shader_source, std::regex{"::"}, "_");
	}

//--------------------------------------------------------------------------------------------------

	void rename_user_main(std::string& shader_source)
	{
		tzslc::transform(shader_source, std::regex{" +main"}, [&](auto beg, auto end)-> std::string
		{
			return " user_main";
		});
	}
	
//--------------------------------------------------------------------------------------------------

	void add_main_definition(std::string& shader_source)
	{
		constexpr char main_def[] = R"|(
void main()
{
	user_main();
	#if TZ_VULKAN
		gl_Position.z = (gl_Position.z + 1.0) * 0.5;
	#endif // TZ_VULKAN
}
		)|";
		shader_source += main_def;
	}	
}
