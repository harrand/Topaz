//
// Created by Harrand on 25/12/2019.
//

#include "test_framework.hpp"
#include "core/tz.hpp"
#include "core/tz_glad/glad_context.hpp"
#include "gl/shader_preprocessor.hpp"
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
	pre.preprocess();
	topaz_expectf(pre.result() == src6_parsed, "tz::gl::p Shader Source Replacement Failed a simple test.\nExpected Source: \n%s\nActual Source: \n%s", pre.result().c_str(), src6_parsed);
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
		tz::terminate();
	}
	return pre.result();
}