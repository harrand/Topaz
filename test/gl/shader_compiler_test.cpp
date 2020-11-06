//
// Created by Harrand on 25/12/2019.
//

#include "test_framework.hpp"
#include "core/core.hpp"
#include "core/tz_glad/glad_context.hpp"
#include "gl/shader_compiler.hpp"
#include "gl/shader.hpp"

tz::test::Case invalid_shader()
{
	tz::test::Case test_case("tz::gl::ShaderCompiler Invalid Source Compilation Tests");
	tz::gl::Shader shader(tz::gl::ShaderType::Vertex);
	std::string src = "gah me can no write shaders very good :(";
	shader.upload_source(src);
	
	tz::gl::ShaderCompiler cmp;
	tz::gl::ShaderCompilerDiagnostic diagnostic = cmp.compile(shader);
	topaz_expect(test_case, !diagnostic.successful(), "tz::gl::ShaderCompiler (default) apparantly managed to compile the shader source \"", src, "\" successfully, which is very worrying");

	return test_case;
}

tz::test::Case valid_shader()
{
	tz::test::Case test_case("tz::gl::ShaderCompiler Valid Source Compilation Tests");
	tz::gl::Shader shader(tz::gl::ShaderType::Vertex);
	std::string src =\
	"#version 430\n\
	\n\
	void main()\n\
	{\n\
	\n\
	}";
	shader.upload_source(src);
	
	tz::gl::ShaderCompiler cmp;
	tz::gl::ShaderCompilerDiagnostic diagnostic = cmp.compile(shader);
	topaz_expect(test_case, diagnostic.successful(), "tz::gl::ShaderCompiler (default) failed compile the shader source \"", src, "\", which I do believe is a valid shader. Info Log: ", diagnostic.get_info_log());

	return test_case;
}

tz::test::Case valid_program()
{
	tz::test::Case test_case("tz::gl::ShaderCompiler Shader Program Compile+Link Tests");
	
	const tz::gl::ShaderCompiler cmp;

	tz::gl::ShaderProgram prg;
	std::string src =\
	"#version 430\n\
	\n\
	void main()\n\
	{\n\
	\n\
	}";

	tz::gl::Shader* v_shader = prg.emplace(tz::gl::ShaderType::Vertex, src);
	tz::gl::Shader* f_shader = prg.emplace(tz::gl::ShaderType::Fragment, src);

	topaz_expect(test_case, v_shader->has_source(), "Didn't upload any sources to the vertex shader!");
	topaz_expect(test_case, f_shader->has_source(), "Didn't upload any sources to the fragment shader!");
	
	// src should be valid for both shader sources. bit of a cheat but hey.
	topaz_expect(test_case, cmp.compile(*v_shader).successful(), "tz::gl::ShaderCompiler failed to compile valid vertex shader...");
	topaz_expect(test_case, cmp.compile(*f_shader).successful(), "tz::gl::ShaderCompiler failed to compile valid fragment shader...");
	topaz_expect_assert(test_case, false, "tz::gl::ShaderProgram compile & link asserted unexpectedly...");
	auto lnk_diag = cmp.link(prg);
	topaz_expect(test_case, lnk_diag.successful(), "tz::gl::ShaderProgram failed to link & validate a successful program. Info log: ", lnk_diag.get_info_log());
	
	topaz_expect(test_case, prg != tz::gl::bound::shader_program(), "tz::gl::ShaderProgram is considered to be bound but we never invoked bind on it!");
	prg.bind();
	topaz_expect_assert(test_case, false, "tz::gl::ShaderProgram unexpectedly asserted while binding -- This is meant to be a valid program...");
	topaz_expect(test_case, prg == tz::gl::bound::shader_program(), "tz::gl::ShaderProrgram was incorrectly considered to not be bound.");
	return test_case;
}

int main()
{
	tz::test::Unit shader;

	// We require topaz to be initialised.
	{
		tz::core::initialise("Shader Compiler Tests", tz::core::invisible_tag);
		shader.add(invalid_shader());
		shader.add(valid_shader());
		shader.add(valid_program());
		tz::core::terminate();
	}
	return shader.result();
}