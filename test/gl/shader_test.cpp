//
// Created by Harrand on 25/12/2019.
//

#include "test_framework.hpp"
#include "core/core.hpp"
#include "core/tz_glad/glad_context.hpp"
#include "gl/shader.hpp"
#include "gl/texture.hpp"

tz::test::Case empty_program()
{
	tz::test::Case test_case("tz::gl::ShaderProgram Emptiness Tests");
	tz::gl::ShaderProgram prg;
	topaz_expect(test_case, !prg.linkable(), "tz::gl::ShaderProgram thinks it is usable, but it is empty!");
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

tz::test::Case attach_texture()
{
	tz::test::Case test_case("tz::gl::ShaderProgram Texture Attachment Tests");
	tz::gl::ShaderProgram prg;
	tz::gl::Texture t;
	topaz_expect(test_case, prg.attached_textures_capacity() > 0, "tz::gl::ShaderProgram wrongly thinks it can't have any texture attachments");
	topaz_expect(test_case, prg.attached_textures_size() == 0, "tz::gl::ShaderProgram wrongly thinks it has at least one texture attached.");
	prg.attach_texture(0, &t, "");
	topaz_expect(test_case, prg.attached_textures_size() == 1, "tz::gl::ShaderProgram had unexpected texture attachments size. Expected ", 1, ", but got ", prg.attached_textures_size());
	prg.detach_texture(0);
	topaz_expect(test_case, prg.attached_textures_size() == 0, "tz::gl::ShaderProgram wrongly thinks it has at least one texture attached.");

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
		shader.add(attach_texture());
		tz::core::terminate();
	}
	return shader.result();
}