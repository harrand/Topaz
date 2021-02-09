#include "core/tz.hpp"
#include "core/debug/print.hpp"
#include "gl/shader.hpp"
#include "gl/shader_preprocessor.hpp"
#include "gl/shader_compiler.hpp"
#include "gl/object.hpp"
#include "gl/buffer.hpp"
#include "gl/frame.hpp"
#include "gl/modules/ssbo.hpp"
#include "render/device.hpp"
#include "GLFW/glfw3.h"

const char *vtx_shader_src = "#version 430\n"
	"layout (location = 0) in uint pos_index;\n"
	"#ssbo position_buffer\n"
	"{\n"
	"   float positions[9];\n"
	"};\n"
	"\n"
	"void main()\n"
	"{\n"
	"   gl_Position = vec4(positions[gl_VertexID*3], positions[(gl_VertexID*3) + 1], positions[(gl_VertexID*3) + 2], 1.0);\n"
	"}\0";
const char *frg_shader_src = "#version 430\n"
	"out vec4 FragColor;\n"
	"void main()\n"
	"{\n"
	"   FragColor = vec4(0.8f, 0.15f, 0.0f, 1.0f);\n"
	"}\n\0";

int main()
{
	// Minimalist Graphics Demo.
	tz::initialise("Topaz TZGLP SSBO Demo");
	{
		tz::gl::Object o;
		tz::gl::p::SSBOModule* ssbo_module = nullptr;

		tz::gl::ShaderPreprocessor pre{vtx_shader_src};
		{
			std::size_t ssbo_module_id = pre.emplace_module<tz::gl::p::SSBOModule>(&o);
			pre.preprocess();
			// Get the module after use.
			ssbo_module = static_cast<tz::gl::p::SSBOModule*>(pre[ssbo_module_id]);
		}
		std::size_t ssbo_id = ssbo_module->get_buffer_id(ssbo_module->size() - 1);
		tz::gl::SSBO* ssbo = o.get<tz::gl::BufferType::ShaderStorage>(ssbo_id);

		tz::gl::ShaderCompiler cpl;
		tz::gl::ShaderProgram prg;
		tz::gl::Shader* vs = prg.emplace(tz::gl::ShaderType::Vertex);
		// We want to upload the preprocessed source.
		vs->upload_source(pre.result());
		tz::gl::Shader* fs = prg.emplace(tz::gl::ShaderType::Fragment);
		fs->upload_source(frg_shader_src);

		cpl.compile(*vs);
		cpl.compile(*fs);
		cpl.link(prg);

		const float vertices[] = {
		-0.5f, -0.5f, 0.0f, // left  
		 0.5f, -0.5f, 0.0f, // right 
		 0.0f,  0.5f, 0.0f  // top   
		};

		ssbo->terminal_resize(sizeof(vertices));
		tz::mem::UniformPool<float> vertex_pool = ssbo->map_uniform<float>();
		for(std::size_t i = 0; i < vertex_pool.capacity(); i++)
			vertex_pool.set(i, vertices[i]);
		auto add_pos = [&vertex_pool](float x, float y, float z)
		{
			vertex_pool[0] += x;
			vertex_pool[3] += x;
			vertex_pool[6] += x;

			vertex_pool[1] += y;
			vertex_pool[4] += y;
			vertex_pool[7] += y;

			vertex_pool[2] += z;
			vertex_pool[5] += z;
			vertex_pool[8] += z;
		};

		unsigned int indices[] = {0, 1, 2};
		std::size_t ibo_id = o.emplace_buffer<tz::gl::BufferType::Index>();
		tz::gl::IBO* ibo = o.get<tz::gl::BufferType::Index>(ibo_id);
		ibo->resize(sizeof(indices));
		ibo->send(indices);

		tz::IWindow& wnd = tz::get().window();
		wnd.register_this();
		wnd.emplace_custom_key_listener([&add_pos](tz::input::KeyPressEvent e)
		{
			switch(e.key)
			{
			case GLFW_KEY_W:
				add_pos(0.0f, 0.05f, 0.0f);
				tz::debug_printf("moving forward.\n");
			break;
			case GLFW_KEY_S:
				add_pos(0.0f, -0.05f, 0.0f);
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
			}
		});

		glClearColor(0.0f, 0.3f, 0.15f, 1.0f);
		tz::render::Device dev{wnd.get_frame(), &prg, &o};
		dev.set_handle(ibo_id);
		while(!wnd.is_close_requested())
		{
			dev.clear();
			o.bind();
			ssbo->bind();
			dev.render();

			wnd.update();
			tz::update();
		}
	}
	tz::terminate();
}