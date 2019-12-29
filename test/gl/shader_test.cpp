//
// Created by Harrand on 25/12/2019.
//

#include "test_framework.hpp"
#include "core/core.hpp"
#include "core/tz_glad/glad_context.hpp"
#include "gl/shader.hpp"

tz::test::Case empty_program()
{
	tz::test::Case test_case("tz::gl::ShaderProgram Emptiness Tests");
	tz::gl::ShaderProgram prg;
	topaz_expect(test_case, !prg.usable(), "tz::gl::ShaderProgram thinks it is usable, but it is empty!");
	return test_case;
}

tz::test::Case empty_shader()
{
	tz::test::Case test_case("tz::gl::Shader Emptiness Tests");
	tz::gl::Shader empty_vertex_shader(tz::gl::ShaderType::Vertex);
	topaz_expect(test_case, !empty_vertex_shader.has_source(), "tz::gl::Shader thinks it has source but it doesn't!");
	empty_vertex_shader.upload_source("#version 430");
	topaz_expect(test_case, empty_vertex_shader.has_source(), "tz::gl::Shader wrongly believes it's empty!");
	return test_case;
}

int main()
{
    tz::test::Unit shader;

    // We require topaz to be initialised.
    {
        tz::core::initialise("Shader Tests");
		shader.add(empty_program());

		shader.add(empty_shader());
        tz::core::terminate();
    }
    return shader.result();
}