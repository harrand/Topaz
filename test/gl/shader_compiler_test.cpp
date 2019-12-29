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

int main()
{
    tz::test::Unit shader;

    // We require topaz to be initialised.
    {
        tz::core::initialise("Shader Compiler Tests");
		shader.add(invalid_shader());
		shader.add(valid_shader());
        tz::core::terminate();
    }
    return shader.result();
}