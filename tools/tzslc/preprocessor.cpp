#include "core/assert.hpp"
#include "preprocessor.hpp"
#include "source_transform.hpp"
#include "gl/api/shader.hpp"

namespace tzslc
{
	constexpr const char* tzslc_platform_defines()
	{
		#if TZ_VULKAN
			return "#define TZ_OGL 0\n#define TZ_VULKAN 1\n";
		#elif TZ_OGL
			return "#define TZ_OGL 1\n#define TZ_VULKAN 0\n";
		#else
			#error "Neither Vulkan nor OGL render-api backend specified";
		#endif
	}

	constexpr const char* tzslc_config_defines()
	{
		#if TZ_DEBUG
			return "#define TZ_DEBUG 1\n";
		#else
			return "#define TZ_DEBUG 0\n";
		#endif
	}

	void add_glsl_header_info(std::string& shader_source)
	{
		std::string hdrinfo = "/*tzslc header info begin*/\n#version 450 core\n";
		hdrinfo += tzslc_platform_defines();
		hdrinfo += tzslc_config_defines();
		hdrinfo += "/*tzslc header info end*/\n";
		shader_source = hdrinfo + shader_source;
	}

	bool evaluate_tzsl_keywords(std::string& shader_source)
	{
		bool done_work = false;
		tzslc::transform(shader_source, std::regex{"tz_VertexID"},
		[&done_work](auto beg, auto end)->std::string
		{
			done_work = true;
			#if TZ_VULKAN
				return "gl_VertexIndex";
			#elif TZ_OGL
				return "gl_VertexID";
			#else
				tz_error("Could not detect render api used during compilation.");
				return "tz_VertexID";
			#endif
		});
		return done_work;
	}

	::tz::gl::ShaderStage preprocess_stage_specifier(std::string& shader_source)
	{
		tz::gl::ShaderStage stage;
		bool found_specifier = false;
		tzslc::transform(shader_source, std::regex{"shader\\(type ?= ?([a-zA-Z]+)\\) ?;"}, [&](auto beg, auto end)-> std::string
		{
			tz_assert(std::distance(beg, end) == 1, "stage(...) is invalid. One string must reside within the parentheses.");
			const std::string& stage_name = *beg;
			found_specifier = true;
			if(stage_name == "vertex")
			{
				stage = tz::gl::ShaderStage::Vertex;
			}
			else if(stage_name == "fragment")
			{
				stage = tz::gl::ShaderStage::Fragment;
			}
			else if(stage_name == "compute")
			{
				stage = tz::gl::ShaderStage::Compute;
			}
			else
			{
				tz_error("Invalid shader stage. Do not recognise \"%s\", must be \"vertex\", \"fragment\" or \"compute\"", stage_name.c_str());
			}
			std::string replacement = "#pragma shader_stage(";
			replacement += stage_name + ")";
			return replacement;
		});
		tz_assert(found_specifier, "Could not find a shader type specifier. You need `shader(type = ?);` to be defined somewhere in the shader source.");
		return stage;
	}

	bool preprocess_outputs(std::string& shader_source)
	{
		tzslc::transform(shader_source, std::regex{"output\\(id ?= ?([0-9]+)\\) (.*)"}, [&](auto beg, auto end)-> std::string
		{
			tz_assert(std::distance(beg, end) == 2, "output(...) parse error.");
			int output_id = std::stoi(*beg);
			std::string rest = *(beg + 1);
			std::string replacement = "layout(location = ";
			replacement += std::to_string(output_id) + ") out ";
			replacement += rest;
			return replacement;
		});
		return true;
	}

	bool preprocess_inputs(std::string& shader_source)
	{
		tzslc::transform(shader_source, std::regex{"input\\(id ?= ?([0-9]+)\\) (.*)"}, [&](auto beg, auto end)-> std::string
		{
			tz_assert(std::distance(beg, end) == 2, "input(...) parse error.");
			int output_id = std::stoi(*beg);
			std::string rest = *(beg + 1);
			std::string replacement = "layout(location = ";
			replacement += std::to_string(output_id) + ") in ";
			replacement += rest;
			return replacement;
		});

		// Also support for flat etc...
		tzslc::transform(shader_source, std::regex{"input\\(id ?= ?([0-9]+), ?([a-zA-Z])\\) (.*)"}, [&](auto beg, auto end)-> std::string
		{
			tz_assert(std::distance(beg, end) == 3, "input(...) parse error.");
			int output_id = std::stoi(*beg);
			std::string extra_specifier = *(beg + 1);
			std::string rest = *(beg + 2);
			std::string replacement = "layout(location = ";
			replacement += std::to_string(output_id) + ")";
			if(extra_specifier == "flat")
			{
				replacement += " flat";
			}
			else
			{
				tz_error("input(id = x, flag): Unrecognised flag \"%s\". Expected \"flat\"", extra_specifier.c_str());
			}
			replacement += " in ";
			replacement += rest;
			return replacement;
		});
		return true;
	}

	void rename_user_main(std::string& shader_source)
	{
		tzslc::transform(shader_source, std::regex{" +main"}, [&](auto beg, auto end)-> std::string
		{
			return " user_main";
		});
	}
	
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

	bool preprocess(PreprocessorModuleField modules, std::string& shader_source, std::string& meta)
	{
		bool done_any_work = evaluate_tzsl_keywords(shader_source);

		if(modules.contains(PreprocessorModule::Assert))
		{
			done_any_work |= preprocess_asserts(shader_source);
		}
		if(modules.contains(PreprocessorModule::DebugPrint))
		{
			done_any_work |= preprocess_prints(shader_source);
		}
		if(modules.contains(PreprocessorModule::Sampler))
		{
			done_any_work |= preprocess_samplers(shader_source, meta);
		}
		tz::gl::ShaderStage stage = preprocess_stage_specifier(shader_source);
		preprocess_outputs(shader_source);
		preprocess_inputs(shader_source);
		add_glsl_header_info(shader_source);
		preprocess_topaz_types(shader_source, meta);

		if(stage == tz::gl::ShaderStage::Vertex)
		{
			rename_user_main(shader_source);
			add_main_definition(shader_source);
		}
		return done_any_work;
	}

	bool preprocess_samplers(std::string& shader_source, std::string& meta)
	{
		#if TZ_VULKAN
			tzslc::transform(shader_source, std::regex{"resource\\(id ?= ?([0-9]+)\\) const texture"}, [&](auto beg, auto end)->std::string
			{
				tz_assert(std::distance(beg, end) == 1, "resource(id = x) const texture <name> : 'x' should be one number");
				int id = std::stoi(*beg);
				std::string replacement = "/*tzslc: const texture*/ layout(binding";
				replacement += " = " + std::to_string(id) + ") uniform sampler2D";
				meta += std::to_string(id) + " = texture\n";
				return replacement;
			});
		#elif TZ_OGL
			bool work_done = false;
			tzslc::transform(shader_source, std::regex{"resource\\(id ?= ?([0-9]+)\\) const texture (.+)\\[([0-9]*)\\]?;"}, [&](auto beg, auto end)->std::string
			{
				tz_assert(std::distance(beg, end) == 3, "resource(id = x) const texture <name>[y] : Failed to parse correctly");
				work_done = true;
				int id = std::stoi(*beg);
				std::string name = *(beg + 1);
				std::string_view arr_len_str = *(beg + 2);
				std::string replacement = "/*tzslc: const texture*/ layout(binding";
				replacement += " = " + std::to_string(id) + ") buffer ImageData\n";
				replacement += "{\n";
				replacement += "\tsampler2D textures[" + std::string(arr_len_str) + "];\n";
				replacement += "} " + name;
				// So because this is now an SSBO we want <name>[x] to now refer to <name>.textures[x]. We do this by adding a super evil #define.
				replacement += ";\n#define " + name + " " + name + ".textures";
				meta += std::to_string(id) + " = texture\n";
				return replacement;
			});
			if(work_done)
			{
				shader_source = "#extension GL_ARB_bindless_texture : enable\n" + shader_source;
			}
		#endif
		return false;
	}

	bool preprocess_prints(std::string& shader_source)
	{
		bool work_done = false;
	   tzslc::transform(shader_source, std::regex{"tz_printf\\((.*)\\).*;"}, [&](auto beg, auto end)->std::string
	   {
	   	std::string contents = *beg;
		// Only actually compile to debug printfs on debug mode.
		#if TZ_DEBUG && TZ_VULKAN
	   		work_done = true;
	   		return std::string("debugPrintfEXT(") + contents + ");";
		#else
			return "";
		#endif
	   });
	   if(work_done)
	   {
		shader_source = std::string("#extension GL_EXT_debug_printf : enable\n") + shader_source;
	   }
	   return false;
	}

	bool preprocess_asserts(std::string& shader_source)
	{
		bool work_done = false;
	   tzslc::transform(shader_source, std::regex{"tz_assert\\((.*),\\s+\"(.*)\""}, [&](auto beg, auto end)->std::string
	   {
		tz_assert(std::distance(beg, end) == 2, "\"tz_assert(<expression>,\"... failed to parse");
		std::string expr = *beg;
		std::string fmt_str = *(beg + 1);
	   	work_done = true;
	   	return std::string("tz_printf(") + std::string("\"") + std::string("TZ_ASSERT_SHADER")+  std::string("[%d]") + fmt_str + "\", " + expr;
	   });
	   if(work_done)
	   {
		shader_source = std::string("#extension GL_EXT_debug_printf : enable\n") + shader_source;
	   }
	   return false;
	}

	bool preprocess_topaz_types(std::string& shader_source, std::string& meta)
	{
		/*
		example tzsl:
		resource(id = 0) const buffer MVP
		{
			mat4 model;
			mat4 view;
			mat4 projection;
		} mvp;

		desired output (vulkan & opengl):
		layout(binding = 0) uniform MVP
		{
			mat4 model;
			mat4 view;
			mat4 projection;
		} mvp;
		*/
	   // Handle 'const buffer' (UBO)
	   tzslc::transform(shader_source, std::regex{"resource\\(id ?= ?([0-9]+)\\) const buffer"}, [&meta](auto beg, auto end)->std::string
	   {
			tz_assert(std::distance(beg, end) == 1, "resource(id = x) const buffer <name> : 'x' should be one number");
			// TODO: Don't hardcode.
			const char* buffer_subtype_name = "ubo";
			int id = std::stoi(*beg);
			std::string replacement = "/*tzslc: const buffer resource (";
			replacement += buffer_subtype_name;
			replacement += ")*/ layout(binding = ";
			replacement += std::to_string(id);
			replacement += ") uniform";

			meta += std::to_string(id) + " = ";
			meta += buffer_subtype_name;
			meta += "\n";
			return replacement;
	   });
	   // Handle 'buffer' (SSBO)
	   tzslc::transform(shader_source, std::regex{"resource\\(id ?= ?([0-9]+)\\) buffer"}, [&meta](auto beg, auto end)->std::string
	   {
			tz_assert(std::distance(beg, end) == 1, "resource(id = x) buffer : 'x' should be one number");
			// TODO: Don't hardcode.
			const char* buffer_subtype_name = "ssbo";
			int id = std::stoi(*beg);
			std::string replacement = "/*tzslc buffer resource (";
			replacement += buffer_subtype_name;
			replacement += ")*/ layout(binding = ";
			replacement += std::to_string(id);
			replacement += ") buffer";
			meta += std::to_string(id) + " = ";
			meta += buffer_subtype_name;
			meta += "\n";
			return replacement;
	   });
	   return false;
	}
}
