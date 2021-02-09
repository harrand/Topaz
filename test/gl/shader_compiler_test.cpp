//
// Created by Harrand on 25/12/2019.
//

#include "test_framework.hpp"
#include "core/tz.hpp"
#include "core/tz_glad/glad_context.hpp"
#include "gl/shader_compiler.hpp"
#include "gl/shader.hpp"

TZ_TEST_BEGIN(invalid_shader)
	tz::gl::Shader shader(tz::gl::ShaderType::Vertex);
	std::string src = "gah me can no write shaders very good :(";
	shader.upload_source(src);
	
	tz::gl::ShaderCompiler cmp;
	tz::gl::ShaderCompilerDiagnostic diagnostic = cmp.compile(shader);
	topaz_expect(!diagnostic.successful(), "tz::gl::ShaderCompiler (default) apparantly managed to compile the shader source \"", src, "\" successfully, which is very worrying");
TZ_TEST_END

TZ_TEST_BEGIN(valid_shader)
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
	topaz_expect(diagnostic.successful(), "tz::gl::ShaderCompiler (default) failed compile the shader source \"", src, "\", which I do believe is a valid shader. Info Log: ", diagnostic.get_info_log());
TZ_TEST_END

TZ_TEST_BEGIN(valid_program)
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

	topaz_expect(v_shader->has_source(), "Didn't upload any sources to the vertex shader!");
	topaz_expect(f_shader->has_source(), "Didn't upload any sources to the fragment shader!");
	
	// src should be valid for both shader sources. bit of a cheat but hey.
	topaz_expect(cmp.compile(*v_shader).successful(), "tz::gl::ShaderCompiler failed to compile valid vertex shader...");
	topaz_expect(cmp.compile(*f_shader).successful(), "tz::gl::ShaderCompiler failed to compile valid fragment shader...");
	topaz_expect_assert(false, "tz::gl::ShaderProgram compile & link asserted unexpectedly...");
	auto lnk_diag = cmp.link(prg);
	topaz_expect(lnk_diag.successful(), "tz::gl::ShaderProgram failed to link & validate a successful program. Info log: ", lnk_diag.get_info_log());
	
	topaz_expect(prg != tz::gl::bound::shader_program(), "tz::gl::ShaderProgram is considered to be bound but we never invoked bind on it!");
	prg.bind();
	topaz_expect_assert(false, "tz::gl::ShaderProgram unexpectedly asserted while binding -- This is meant to be a valid program...");
	topaz_expect(prg == tz::gl::bound::shader_program(), "tz::gl::ShaderProrgram was incorrectly considered to not be bound.");
TZ_TEST_END

int main()
{
	tz::test::Unit shader;

	// We require topaz to be initialised.
	{
		tz::initialise("Shader Compiler Tests", tz::invisible_tag);
		shader.add(invalid_shader());
		shader.add(valid_shader());
		shader.add(valid_program());
		tz::terminate();
	}
	return shader.result();
}