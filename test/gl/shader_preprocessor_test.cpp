//
// Created by Harrand on 25/12/2019.
//

#include "test_framework.hpp"
#include "core/core.hpp"
#include "core/tz_glad/glad_context.hpp"
#include "gl/shader_preprocessor.hpp"
#include <cctype>

constexpr char src[] =\
	"#version 430\n\
	\n\
	void main()\n\
	{\n\
	\n\
	}";

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

tz::test::Case no_modules()
{
	tz::test::Case test_case("tz::gl::ShaderPreprocessor Empty Tests");
	tz::gl::ShaderPreprocessor pre{src};
	topaz_expect(test_case, pre.empty(), "tz::gl::ShaderPreprocessor constructed with mere source failed to be empty! Size: ", pre.size());
	pre.preprocess();
	topaz_expect_assert(test_case, false, "tz::gl::ShaderPreprocessor::preprocess(): Asserted even though we don't even have any modules!");
	topaz_expect(test_case, pre.result() == src, "tz::gl::ShaderPreprocessor::preprocess(): Made changes to the source even though there aren't any modules! Before: \n\"", src, "\"\nAfter:\"", pre.result(), "\"");
	return test_case;
}

tz::test::Case example_module()
{
	tz::test::Case test_case("tz::gl::ShaderPreprocessor Example Module Tests (Uppercase TZGLP)");
	tz::gl::ShaderPreprocessor pre{src};
	pre.emplace_module<tz::test::TestUppercaseModule>();
	pre.preprocess();
	// All chars that can be upper-case should be upper.
	for(char c : pre.result())
	{
		topaz_expect(test_case, !std::islower(c), "tz::gl::p Module TestUppercase Failed: Found non-uppercase character '", c, "'");
	}
	return test_case;
}

tz::test::Case module_order()
{
	tz::test::Case test_case("tz::gl::ShaderPreprocessor Module Ordering Tests");
	tz::gl::ShaderPreprocessor pre{src};
	pre.emplace_module<tz::test::TestUppercaseModule>();
	pre.emplace_module<tz::test::TestLowercaseModule>();
	pre.preprocess();
	// All chars that can be upper-case should be lower. All alphabetic chars should be uppered by the 0th module, but lowered again by the 1st module.
	// Module order must matter!
	for(char c : pre.result())
	{
		topaz_expect(test_case, !std::isupper(c), "tz::gl::p Module Ordering Failed: Found non-lowercase character '", c, "'");
	}
	return test_case;
}

int main()
{
    tz::test::Unit pre;

    // We require topaz to be initialised.
    {
        tz::core::initialise("Shader Preprocessor Tests");
		pre.add(no_modules());
		pre.add(example_module());
		pre.add(module_order());
        tz::core::terminate();
    }
    return pre.result();
}