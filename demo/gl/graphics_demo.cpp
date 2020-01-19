#include "core/core.hpp"
#include "core/debug/print.hpp"
#include "geo/matrix_transform.hpp"
#include "gl/tz_stb_image/image_reader.hpp"
#include "gl/shader.hpp"
#include "gl/shader_compiler.hpp"
#include "gl/object.hpp"
#include "gl/buffer.hpp"
#include "gl/frame.hpp"
#include "gl/modules/ubo.hpp"
#include "gl/texture.hpp"
#include "render/device.hpp"
#include "GLFW/glfw3.h"

const char *vertexShaderSource = "#version 430\n"
    "layout (location = 1) in vec3 aPos;\n"
	"layout (location = 2) in vec2 aTexcoord;\n"
	"#ubo model_matrices\n"
	"{\n"
	"	mat4 model;\n"
	"};\n"
	"out vec2 texcoord;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = model * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
	"	texcoord = aTexcoord;\n"
    "}\0";
const char *fragmentShaderSource = "#version 430\n"
    "out vec4 FragColor;\n"
	"in vec2 texcoord;\n"
	"uniform sampler2D checkerboard;\n"
    "void main()\n"
    "{\n"
	"	FragColor = texture(checkerboard, texcoord);\n"
    "}\n\0";

int main()
{
	// Minimalist Graphics Demo.
	tz::core::initialise("Topaz Graphics Demo");
	{
		tz::gl::Object o;
		tz::gl::p::UBOModule* ubo_module = nullptr;
		tz::gl::ShaderPreprocessor pre{vertexShaderSource};
		{
			std::size_t ubo_module_id = pre.emplace_module<tz::gl::p::UBOModule>(&o);
			pre.preprocess();
			ubo_module = static_cast<tz::gl::p::UBOModule*>(pre[ubo_module_id]);
		}
		std::size_t ubo_id = ubo_module->get_buffer_id(ubo_module->size() - 1);
		tz::gl::UBO* ubo = o.get<tz::gl::BufferType::UniformStorage>(ubo_id);
		ubo->terminal_resize(sizeof(tz::Mat4));
		tz::mem::UniformPool<tz::Mat4> model_matrix = ubo->map_pool<tz::Mat4>();

		tz::gl::ShaderCompiler cpl;
		tz::gl::ShaderProgram prg;
		tz::gl::Shader* vs = prg.emplace(tz::gl::ShaderType::Vertex);
		vs->upload_source(pre.result());
		tz::gl::Shader* fs = prg.emplace(tz::gl::ShaderType::Fragment);
		fs->upload_source(fragmentShaderSource);

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

		prg.attach_texture(0, &checkerboard, "checkerboard");

		std::size_t vbo_id = o.emplace_buffer<tz::gl::BufferType::Array>();
		tz::gl::VBO* vbo = o.get<tz::gl::BufferType::Array>(vbo_id);
		std::size_t texcoords_id = o.emplace_buffer<tz::gl::BufferType::Array>();
		tz::gl::VBO* texcoord_vbo = o.get<tz::gl::BufferType::Array>(texcoords_id);
		texcoord_vbo->resize(sizeof(texcoords));
		texcoord_vbo->send(texcoords);

		vbo->terminal_resize(sizeof(vertices));
		tz::mem::UniformPool<float> vertex_pool = vbo->map_pool<float>();
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

		o.format(vbo_id, tz::gl::fmt::three_floats);
		o.format(texcoords_id, tz::gl::fmt::two_floats);
		std::size_t ibo_id = o.emplace_buffer<tz::gl::BufferType::Index>();
		tz::gl::IBO* ibo = o.get<tz::gl::BufferType::Index>(ibo_id);
		ibo->resize(3 * sizeof(unsigned int));
		unsigned int indices[] = {0, 1, 2};
		ibo->send(indices);

		float rotation_x = 0.0f;
		float rotation_y = 0.0f;

		tz::core::IWindow& wnd = tz::core::get().window();
		wnd.register_this();
		wnd.emplace_custom_key_listener([&add_pos, &rotation_y](tz::input::KeyPressEvent e)
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
			rotation_x += 0.02f;
			rotation_y += 0.02f;

        	dev.clear();
			o.bind();
			model_matrix.set(0, tz::geo::rotate(tz::Vec3{{0.0f, rotation_y, rotation_x}}));
			ubo->bind();
			dev.render();
			wnd.update();
			tz::core::update();
		}
	}
	tz::core::terminate();
}