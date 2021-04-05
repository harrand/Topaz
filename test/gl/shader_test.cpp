//
// Created by Harrand on 25/12/2019.
//

#include "test_framework.hpp"
#include "core/tz.hpp"
#include "ext/tz_glad/glad_context.hpp"
#include "gl/shader.hpp"
#include "gl/texture.hpp"

TZ_TEST_BEGIN(empty_program)
	tz::gl::ShaderProgram prg;
	topaz_expect(!prg.linkable(), "tz::gl::ShaderProgram thinks it is usable, but it is empty!");
TZ_TEST_END

TZ_TEST_BEGIN(empty_shader)
	tz::gl::Shader empty_vertex_shader(tz::gl::ShaderType::Vertex);
	topaz_expect(!empty_vertex_shader.has_source(), "tz::gl::Shader thinks it has source but it doesn't!");
	empty_vertex_shader.upload_source("#version 430");
	topaz_expect(empty_vertex_shader.has_source(), "tz::gl::Shader wrongly believes it's empty!");
TZ_TEST_END

TZ_TEST_BEGIN(attach_texture)
	tz::gl::ShaderProgram prg;
	tz::gl::Texture t;
	topaz_expect(prg.attached_textures_capacity() > 0, "tz::gl::ShaderProgram wrongly thinks it can't have any texture attachments");
	topaz_expect(prg.attached_textures_size() == 0, "tz::gl::ShaderProgram wrongly thinks it has at least one texture attached.");
	prg.attach_texture(0, &t, "");
	topaz_expect(prg.attached_textures_size() == 1, "tz::gl::ShaderProgram had unexpected texture attachments size. Expected ", 1, ", but got ", prg.attached_textures_size());
	prg.detach_texture(0);
	topaz_expect(prg.attached_textures_size() == 0, "tz::gl::ShaderProgram wrongly thinks it has at least one texture attached.");
TZ_TEST_END

int main()
{
	tz::test::Unit shader;

	// We require topaz to be initialised.
	{
		tz::initialise("Shader Tests", tz::invisible_tag);
		shader.add(empty_program());
		shader.add(empty_shader());
		shader.add(attach_texture());
		tz::terminate();
	}
	return shader.result();
}