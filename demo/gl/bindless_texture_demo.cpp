#include "core/tz.hpp"
#include "core/debug/print.hpp"
#include "core/matrix_transform.hpp"
#include "ext/tz_stb_image/image_reader.hpp"
#include "dui/window.hpp"
#include "gl/shader.hpp"
#include "gl/shader_compiler.hpp"
#include "gl/manager.hpp"
#include "gl/buffer.hpp"
#include "gl/frame.hpp"
#include "gl/modules/ubo.hpp"
#include "gl/modules/bindless_sampler.hpp"
#include "gl/texture.hpp"
#include "render/device.hpp"
#include "GLFW/glfw3.h"

const char* vtx_shader_src = R"GLSL(
	#version 430
	layout (location = 0) in vec3 aPos;
	layout (location = 1) in vec2 aTexcoord;
	#ubo matrices
	{
		mat4 mvp;
	};
	out vec2 texcoord;
	void main()
	{
		gl_Position = mvp * vec4(aPos.x, aPos.y, aPos.z, 1.0);
		texcoord = aTexcoord;
	})GLSL";
const char* frg_shader_src = R"GLSL(
	#version 430
	#extension GL_ARB_bindless_texture : require
	out vec4 FragColor;
	in vec2 texcoord;
	#ubo bricks
	{
		tz_bindless_sampler bricks_sampler;
	};
	void main()
	{
		FragColor = texture(bricks_sampler, texcoord);
	})GLSL";

int main()
{
	// Minimalist Graphics Demo.
	tz::initialise("Topaz Bindless Texture Demo");
	{
		tz::get().render_settings().set_culling(tz::RenderSettings::CullTarget::Nothing);
		tz::gl::Object o;
		tz::dui::track_object(&o);
		tz::gl::p::UBOModule* ubo_module = nullptr;
		std::string preprocess_vertex_source;
		tz::gl::ShaderPreprocessor pre{vtx_shader_src};
		{
			std::size_t ubo_module_id = pre.emplace_module<tz::gl::p::UBOModule>(o);
			pre.emplace_module<tz::gl::p::BindlessSamplerModule>();
			pre.preprocess();
			preprocess_vertex_source = pre.result();
			pre.set_source(frg_shader_src);
			pre.preprocess();
			ubo_module = static_cast<tz::gl::p::UBOModule*>(pre[ubo_module_id]);
		}
		std::size_t ubo_id = ubo_module->get_buffer_id(0);
		std::size_t bricks_ubo_id = ubo_module->get_buffer_id(1);
		tz::gl::UBO* ubo = o.get<tz::gl::BufferType::UniformStorage>(ubo_id);
		ubo->terminal_resize(sizeof(tz::Mat4) * 3);
		tz::mem::UniformPool<tz::Mat4> matrix = ubo->map_uniform<tz::Mat4>();

		tz::gl::ShaderCompiler cpl;
		tz::gl::ShaderProgram prg;
		tz::gl::Shader* vs = prg.emplace(tz::gl::ShaderType::Vertex);
		vs->upload_source(preprocess_vertex_source);
		tz::gl::Shader* fs = prg.emplace(tz::gl::ShaderType::Fragment);
		fs->upload_source(pre.result());

		cpl.compile(*vs);
		cpl.compile(*fs);
		cpl.link(prg);

		const float vertices[] = {
		-0.5f, -0.5f, 0.0f, // left  
		 0.5f, -0.5f, 0.0f, // right 
		 0.0f,  0.5f, 0.0f  // top   
		};

		const float texcoords[] = {
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.5f, 1.0f,
		};

		auto rgba_checkerboard = tz::ext::stb::read_image<tz::gl::PixelRGB8>("res/textures/bricks.jpg");
		tz::gl::Texture checkerboard;
		checkerboard.set_parameters(tz::gl::default_texture_params);
		checkerboard.set_data(rgba_checkerboard);

		//prg.attach_texture(0, &checkerboard, "checkerboard");
		checkerboard.make_terminal();
		tz::gl::BindlessTextureHandle check_handle = checkerboard.get_terminal_handle();
		// Now fill the thing with just the terminal handle. Should work perfectly.
		tz::gl::UBO* bricks_ubo = o.get<tz::gl::BufferType::UniformStorage>(bricks_ubo_id);
		bricks_ubo->resize(sizeof(tz::gl::BindlessTextureHandle));
		bricks_ubo->send(&check_handle);

		std::size_t vbo_id = o.emplace_buffer<tz::gl::BufferType::Array>();
		tz::gl::VBO* vbo = o.get<tz::gl::BufferType::Array>(vbo_id);
		std::size_t texcoords_id = o.emplace_buffer<tz::gl::BufferType::Array>();
		tz::gl::VBO* texcoord_vbo = o.get<tz::gl::BufferType::Array>(texcoords_id);
		texcoord_vbo->resize(sizeof(texcoords));
		texcoord_vbo->send(texcoords);

		vbo->resize(sizeof(vertices));
		vbo->send(vertices);
		tz::Vec3 triangle_pos{{0.0f, 0.0f, 0.0f}};
		auto add_pos = [&triangle_pos](float x, float y, float z)
		{
			triangle_pos[0] += x;
			triangle_pos[1] += y;
			triangle_pos[2] += z;
		};

		o.format(vbo_id, tz::gl::fmt::three_floats);
		o.format(texcoords_id, tz::gl::fmt::two_floats);
		std::size_t ibo_id = o.emplace_buffer<tz::gl::BufferType::Index>();
		tz::gl::IBO* ibo = o.get<tz::gl::BufferType::Index>(ibo_id);
		ibo->resize(3 * sizeof(unsigned int));
		unsigned int indices[] = {0, 1, 2};
		ibo->send(indices);

		float rotation_y = 0.0f;

		tz::IWindow& wnd = tz::get().window();
		wnd.register_this();
		wnd.emplace_custom_key_listener([&add_pos, &rotation_y](tz::input::KeyPressEvent e)
		{
			switch(e.key)
			{
			case GLFW_KEY_W:
				add_pos(0.0f, 0.00f, -0.05f);
				tz::debug_printf("moving forward.\n");
			break;
			case GLFW_KEY_S:
				add_pos(0.0f, 0.00f, 0.05f);
				tz::debug_printf("moving backward.\n");
			break;
			case GLFW_KEY_A:
				add_pos(-0.05f, 0.0f, 0.0f);
				tz::debug_printf("moving left\n");
			break;
			case GLFW_KEY_D:
				add_pos(0.05f, 0.0f, 0.0f);
				tz::debug_printf("moving right\n");
			break;
			case GLFW_KEY_SPACE:
				add_pos(0.0f, 0.05f, 0.0f);
				tz::debug_printf("moving up\n");
			break;
			case GLFW_KEY_LEFT_SHIFT:
				add_pos(0.0f, -0.05f, 0.0f);
				tz::debug_printf("moving down\n");
			break;
			}
		});

		glClearColor(0.0f, 0.3f, 0.15f, 1.0f);
		tz::render::Device dev{wnd.get_frame(), &prg, &o};
		dev.set_handle(ibo_id);
		while(!wnd.is_close_requested())
		{
			rotation_y += 0.02f;

			dev.clear();
			o.bind();
			bricks_ubo->bind();
			tz::Mat4 m = tz::model(triangle_pos, tz::Vec3{{0.0f, rotation_y, 0.0f}}, tz::Vec3{{1.0f, 1.0f, 1.0f}});
			tz::Mat4 v = tz::view(tz::Vec3{{0.0f, 0.0f, 5.0f}}, tz::Vec3{{0.0f, 0.0f, 0.0f}});
			tz::Mat4 p = tz::perspective(1.57f, 1920.0f/1080.0f, 0.1f, 1000.0f);
			matrix.set(0, p * v * m);
			ubo->bind();
			dev.render();
			tz::update();
			wnd.update();
		}
	}
	tz::terminate();
}