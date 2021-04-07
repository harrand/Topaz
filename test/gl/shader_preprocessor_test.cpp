//
// Created by Harrand on 25/12/2019.
//

#include "test_framework.hpp"
#include "core/tz.hpp"
#include "ext/tz_glad/glad_context.hpp"
#include "gl/shader_preprocessor.hpp"
#include "gl/modules/cplvals.hpp"
#include "gl/modules/include.hpp"
#include "gl/modules/ssbo.hpp"
#include "gl/modules/ubo.hpp"
#include "gl/object.hpp"
#include <cctype>

constexpr char src[] =\
	"#version 430\n\
	\n\
	void main()\n\
	{\n\
	\n\
	}";

constexpr char src2[] =\
	"#version 430\n\
	\n\
	#include \"include_me.header.glsl\"\n\
	void main()\n\
	{\n\
	\n\
	}";

constexpr char src3[] =\
	"#version 430\n\
	\n\
	#include \"include_me.header.glsl\"\n\
	#include \"include_me2.header.glsl\"\n\
	\n\
	#include \"include_me3.header.glsl\"\n\
	void main()\n\
	{\n\
	\n\
	}";

constexpr char src4[] =\
	"#version 430\n\
	\n\
	#ssbo wabbadabbadoo\n\
	{\n\
\n\
	}\n\
	void main()\n\
	{\n\
	\n\
	}";

constexpr char src5[] =\
	"#version 430\n\
	\n\
	#ubo wabbadabbadoo\n\
	{\n\
\n\
	}\n\
	void main()\n\
	{\n\
	\n\
	}";

const char* src6 = R"glsl(
	#version 430
	#ubo wabbadabbadoo
	{

	};

	#ubo wabbadabbadoo
	{

	};

	#ssbo bobby_b
	{


	};

	#ssbo bobby_b
	{

	};

	void main()
	{
		// yee
	}
)glsl";

const char* src6_parsed = R"glsl(
	#version 430
	layout(std140, binding = 0) uniform wabbadabbadoo
	{

	};

	layout(std140, binding = 0) uniform wabbadabbadoo
	{

	};

	layout(std430, binding = 1) buffer bobby_b
	{


	};

	layout(std430, binding = 1) buffer bobby_b
	{

	};

	void main()
	{
		// yee
	}
)glsl";

const char* src7 = R"glsl(
	void dub(int x)
	{
		return x * 2;
	}

	void main()
	{
		int x = #@seven;
		x = dub(#@seven * 2);
	}
)glsl";

const char* src7_parsed = R"glsl(
	void dub(int x)
	{
		return x * 2;
	}

	void main()
	{
		int x = 7;
		x = dub(7 * 2);
	}
)glsl";

namespace tz::test
{
	class TestUppercaseModule : public tz::gl::p::IModule
	{
		virtual void operator()(std::string& source) const override
		{
			for(char& c : source)
				c = std::toupper(c);
		}
	};

	class TestLowercaseModule : public tz::gl::p::IModule
	{
		virtual void operator()(std::string& source) const override
		{
			for(char& c : source)
				c = std::tolower(c);
		}
	};
}

TZ_TEST_BEGIN(no_modules)
	tz::gl::ShaderPreprocessor pre{src};
	topaz_expect(pre.empty(), "tz::gl::ShaderPreprocessor constructed with mere source failed to be empty! Size: ", pre.size());
	pre.preprocess();
	topaz_expect_assert(false, "tz::gl::ShaderPreprocessor::preprocess(): Asserted even though we don't even have any modules!");
	topaz_expect(pre.result() == src, "tz::gl::ShaderPreprocessor::preprocess(): Made changes to the source even though there aren't any modules! Before: \n\"", src, "\"\nAfter:\"", pre.result(), "\"");
TZ_TEST_END

TZ_TEST_BEGIN(example_module)
	tz::gl::ShaderPreprocessor pre{src};
	pre.emplace_module<tz::test::TestUppercaseModule>();
	pre.preprocess();
	// All chars that can be upper-case should be upper.
	for(char c : pre.result())
	{
		topaz_expect(!std::islower(c), "tz::gl::p Module TestUppercase Failed: Found non-uppercase character '", c, "'");
	}
TZ_TEST_END

TZ_TEST_BEGIN(module_order)
	tz::gl::ShaderPreprocessor pre{src};
	std::size_t u = pre.emplace_module<tz::test::TestUppercaseModule>();
	std::size_t l = pre.emplace_module<tz::test::TestLowercaseModule>();
	pre.preprocess();
	// All chars that can be upper-case should be lower. All alphabetic chars should be uppered by the 0th module, but lowered again by the 1st module.
	// Module order must matter!
	for(char c : pre.result())
	{
		topaz_expect(!std::isupper(c), "tz::gl::p Module Ordering Failed: Found non-lowercase character '", c, "'");
	}
	// Now swap modules!
	pre.swap_modules(u, l);
	pre.preprocess();
	// This time, the uppercase happens last, so everything should be upper now!
	for(char c : pre.result())
	{
		topaz_expect(!std::islower(c), "tz::gl::p Module Ordering Failed: Found non-uppercase character '", c, "'");
	}
TZ_TEST_END

TZ_TEST_BEGIN(include_file)
	// Easy test-case.
	{
		tz::gl::ShaderPreprocessor pre{src2};
		pre.emplace_module<tz::gl::p::IncludeModule>(__FILE__);
		// src2 includes "include_me.header.glsl" which is guaranteed to be in the same directory as this source file.
		// it is also guaranteed to contain the string "Plums"
		// if the preprocessed source successfully contains the string "Plums", then we will know the include worked.
		pre.preprocess();
		std::size_t find_result = pre.result().find("Plums");
		topaz_expect(find_result != std::string::npos, "tz::gl::p::IncludeModule failed to process include correctly. Preprocessed source: \n\"", pre.result(), "\"");
	}
	// Might have got lucky with a singular include, let's try three!
	{
		// src3 includes "include_me2" and "include_me3", which respectively contain the strings "Apples" and "Oranges".
		tz::gl::ShaderPreprocessor pre{src3};
		pre.emplace_module<tz::gl::p::IncludeModule>(__FILE__);
		pre.preprocess();
		std::size_t a = pre.result().find("Plums");
		std::size_t b = pre.result().find("Apples");
		std::size_t c = pre.result().find("Oranges");
		topaz_expect(a != std::string::npos, "tz::gl::p::IncludeModule failed to process include correctly. Preprocessed source: \n\"", pre.result(), "\"");
		topaz_expect(b != std::string::npos, "tz::gl::p::IncludeModule failed to process include correctly. Preprocessed source: \n\"", pre.result(), "\"");
		topaz_expect(c != std::string::npos, "tz::gl::p::IncludeModule failed to process include correctly. Preprocessed source: \n\"", pre.result(), "\"");
	}
TZ_TEST_END

TZ_TEST_BEGIN(defined_ssbo)
	tz::gl::Object o;
		
	tz::gl::ShaderPreprocessor pre{src4};
	std::size_t ssbo_module_id = pre.emplace_module<tz::gl::p::SSBOModule>(o);
	pre.preprocess();

	// src4 contains a ssbo called "wabbadabbadoo".
	// get the SSBOModule, check that it has an ssbo with that name.
	// check the corresponding ssbo_id and ensure that the layout qualifier ids match.
	tz::gl::p::IModule* module = pre[ssbo_module_id];
	auto* ssbo_module = static_cast<tz::gl::p::SSBOModule*>(module);
	// We should have exactly one entry.
	topaz_expect(ssbo_module->size() == 1, "tz::gl::p::SSBOModule::size(): Had unexpected value. Expected ", 1, ", got ", ssbo_module->size());
	// Let's get that entry!
	std::size_t ssbo_id = ssbo_module->get_buffer_id(0);
	o.get<tz::gl::BufferType::ShaderStorage>(ssbo_id);
	topaz_expect_assert(false, "Unexpected assert after getting the SSBO from the tzglp module...");
TZ_TEST_END

TZ_TEST_BEGIN(defined_ubo)
	tz::gl::Object o;
		
	tz::gl::ShaderPreprocessor pre{src5};
	std::size_t ubo_module_id = pre.emplace_module<tz::gl::p::UBOModule>(o);
	pre.preprocess();

	// src4 contains a ubo called "wabbadabbadoo".
	// get the UBOModule, check that it has an ubo with that name.
	// check the corresponding ubo_id and ensure that the layout qualifier ids match.
	tz::gl::p::IModule* module = pre[ubo_module_id];
	auto* ubo_module = static_cast<tz::gl::p::UBOModule*>(module);
	// We should have exactly one entry.
	topaz_expect(ubo_module->size() == 1, "tz::gl::p::UBOModule::size(): Had unexpected value. Expected ", 1, ", got ", ubo_module->size());
	// Let's get that entry!
	std::size_t ubo_id = ubo_module->get_buffer_id(0);
	o.get<tz::gl::BufferType::UniformStorage>(ubo_id);
	topaz_expect_assert(false, "Unexpected assert after getting the UBO from the tzglp module...");
TZ_TEST_END

TZ_TEST_BEGIN(shader_buffer_name_clash)
	tz::gl::Object o;

	tz::gl::ShaderPreprocessor pre{src6};
	std::size_t ubo_module_id = pre.emplace_module<tz::gl::p::UBOModule>(o);
	std::size_t ssbo_module_id = pre.emplace_module<tz::gl::p::SSBOModule>(o);
	pre.preprocess();
	tz::gl::p::IModule* module = pre[ubo_module_id];
	tz::gl::p::IModule* ssbomod = pre[ssbo_module_id];
	auto* ubo_module = static_cast<tz::gl::p::UBOModule*>(module);
	auto* ssbo_module = static_cast<tz::gl::p::SSBOModule*>(ssbomod);
	// Should have one entry. src6 has 2 references to the same UBO.
	topaz_expectf(ubo_module->size() == 1, "tz::gl::p::UBOModule::size(): Had unexpected value (two ubos of the same name). Expected %d, got %zu", 1, ubo_module->size());
	// Same story with ssbo
	topaz_expectf(ssbo_module->size() == 1, "tz::gl::p::SSBOmodule::size(): Had unexpected value (two ssbos of the same name). Expected %d, got %zu", 1, ssbo_module->size());
TZ_TEST_END

TZ_TEST_BEGIN(shader_source_replacement_correct)
	tz::gl::Object o;

	tz::gl::ShaderPreprocessor pre{src6};
	pre.emplace_module<tz::gl::p::UBOModule>(o);
	pre.emplace_module<tz::gl::p::SSBOModule>(o);
	std::size_t ctvm_id = pre.emplace_module<tz::gl::p::CompileTimeValueModule>();
	auto* ctvm = static_cast<tz::gl::p::CompileTimeValueModule*>(pre[ctvm_id]);
	pre.preprocess();
	topaz_expectf(pre.result() == src6_parsed, "tz::gl::p Shader Source Replacement Failed a simple test.\nExpected Source: \n%s\nActual Source: \n%s", pre.result().c_str(), src6_parsed);
	pre.set_source(src7);

	ctvm->set("seven", "7");
	pre.preprocess();
	topaz_expectf(pre.result() == src7_parsed, "tz::gl::p Shader Source Replacement Failed a simple test.\nExpected Source: \n%s\nActual Source: \n%s", pre.result().c_str(), src7_parsed);
TZ_TEST_END

const char* src8_parsed = R"glsl(
struct PointLight
{
    vec3 position;
    vec3 colour;
    float power;
};

struct DirectionalLight
{
    vec3 direction;
    vec3 colour;
    float power;
};

/**
 * Convert a point-light to a directional-light. This can only be performed in camera-space.
 */
DirectionalLight point_to_directional(PointLight light_cameraspace, vec3 frag_position_cameraspace)
{
    vec3 eye_dir = vec3(0, 0, 0) - frag_position_cameraspace;
    DirectionalLight dir;
    dir.direction = light_cameraspace.position + eye_dir;
    dir.colour = light_cameraspace.colour;
    dir.power = light_cameraspace.power;
    return dir;
}

float lambert(DirectionalLight light, vec3 normal)
{
    return clamp(dot(light.direction, normal), 0.0, 1.0);
}

vec3 lambertian_diffuse(DirectionalLight light, vec3 diffuse_colour, vec3 normal)
{
    float lambertian = lambert(light, normal);
    return diffuse_colour * light.colour * light.power * lambertian;
}

/**
* Blinn-Phong Reflection Model against a directional light.
* Precondition: The space of the light direction matches that of the normal. Otherwise, inaccurate results will be obtained.
*/
vec3 blinnphong_specular(DirectionalLight light, vec3 specular_colour, vec3 normal, vec3 vertex_pos)
{
    float lambertian = lambert(light, normal);
    if(lambertian > 0.0)
    {
        vec3 view_dir = normalize(-vertex_pos);
        vec3 half_dir = normalize(light.direction + view_dir);
        float spec_angle = clamp(dot(half_dir, normal), 0.0, 1.0);
        float specular = pow(spec_angle, 4.0);
        return specular_colour * light.colour * light.power * specular;
    }
    return vec3(0.0, 0.0, 0.0);
}

DirectionalLight white_light()
{
    return DirectionalLight(vec3(), vec3(), 1.0);
}
)glsl";

const char* src8 = R"glsl(
#include "include_me4.header.glsl"
)glsl";

TZ_TEST_BEGIN(multi_pass_include)
	tz::gl::ShaderPreprocessor pre{src8};
	pre.emplace_module<tz::gl::p::IncludeModule>(tz::res().get_path() + "/test/gl/a");

	pre.preprocess();
	topaz_expectf(pre.result() == src8_parsed, "multi_pass_include failed. Expected result:\n\"%s\"\nbut got\n\"%s\"", pre.result().c_str(), src8_parsed);
TZ_TEST_END

int main()
{
	tz::test::Unit pre;

	// We require topaz to be initialised.
	{
		tz::initialise("Shader Preprocessor Tests", tz::invisible_tag);
		pre.add(no_modules());
		pre.add(example_module());
		pre.add(module_order());
		pre.add(include_file());
		pre.add(defined_ssbo());
		pre.add(defined_ubo());
		pre.add(shader_buffer_name_clash());
		pre.add(shader_source_replacement_correct());
		pre.add(multi_pass_include());
		tz::terminate();
	}
	return pre.result();
}