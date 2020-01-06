#include "core/core.hpp"
#include "core/debug/print.hpp"
#include "gl/shader.hpp"
#include "gl/shader_compiler.hpp"
#include "gl/object.hpp"
#include "gl/buffer.hpp"
#include "gl/frame.hpp"
#include "gl/modules/ubo.hpp"
#include "gl/texture.hpp"
#include "render/pipeline.hpp"
#include "GLFW/glfw3.h"

const char *vertexShaderSource = "#version 430\n"
    "layout (location = 0) in vec3 aPos;\n"
	"layout (location = 1) in vec2 aTexcoord;\n"
	"out vec2 texcoord;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
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
	// This will bind an initial checkerboard texture to a triangle.
	// All render invocations will also write to the texture called 'render_to_me'.
	// The texture bound to the triangle will be replaced with the texture, which was of the entire window.
	// Because the screen is mostly the background colour (green), you will find that the triangle appears to very quickly shrink into nothingness.
	// This is not true however; the size of the triangle is completely unchanged; only the texture used is changing!

	tz::core::initialise("Topaz Postprocess Demo");
	{
		tz::gl::Object o;

		tz::gl::ShaderCompiler cpl;
		tz::gl::ShaderProgram prg;
		tz::gl::Shader* vs = prg.emplace(tz::gl::ShaderType::Vertex);
		vs->upload_source(vertexShaderSource);
		tz::gl::Shader* fs = prg.emplace(tz::gl::ShaderType::Fragment);
		fs->upload_source(fragmentShaderSource);

		cpl.compile(*vs);
		cpl.compile(*fs);
		cpl.link(prg);

        tz::gl::Frame second_target{1920, 1080};
        tz::gl::Texture& render_to_me = second_target.emplace_texture(GL_COLOR_ATTACHMENT0);
        {
            // Render-to-texture details.
            tz::gl::TextureDataDescriptor desc{GL_UNSIGNED_BYTE, GL_RGBA8, GL_RGBA, 1920, 1080};
            tz::gl::TextureDataDescriptor depth_desc{GL_FLOAT, GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, 1920, 1080};
            second_target.emplace_renderbuffer(GL_DEPTH_ATTACHMENT, depth_desc);
            render_to_me.resize(desc);
        }

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

		tz::gl::PixelRGBA8 black_pixel{std::byte{}, std::byte{}, std::byte{}, std::byte{255}};
		tz::gl::PixelRGBA8 white_pixel{std::byte{255}, std::byte{255}, std::byte{255}, std::byte{255}};
		tz::gl::Image<tz::gl::PixelRGBA8> rgba_checkerboard{2, 2};
		rgba_checkerboard(0, 0) = black_pixel;
		rgba_checkerboard(1, 0) = white_pixel;
		rgba_checkerboard(0, 1) = white_pixel;
		rgba_checkerboard(1, 1) = black_pixel;
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

		tz::core::IWindow& wnd = tz::core::get().window();
		wnd.emplace_custom_key_listener([&add_pos, &checkerboard, &rgba_checkerboard](tz::input::KeyPressEvent e)
		{
			if(e.action == GLFW_PRESS)
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
				case GLFW_KEY_G:
					checkerboard.set_data(rgba_checkerboard);
				break;
				}
			}
		});

		glClearColor(0.0f, 0.3f, 0.15f, 1.0f);
		tz::render::Device dev{wnd.get_frame(), &prg, &o};
        tz::render::Device dev2{&second_target, &prg, &o};
		dev.set_handle(ibo_id);
        dev2.set_handle(ibo_id);
		tz::render::Pipeline pipeline;
		pipeline.add(dev);
		pipeline.add(dev2);
		while(!wnd.is_close_requested())
		{
        	pipeline.clear();
			pipeline.render();
            {
                tz::gl::Image<tz::gl::PixelRGBA8> img = render_to_me.get_data<tz::gl::PixelRGBA, std::byte>();
                checkerboard.set_data(img);
            }

			wnd.update();
			tz::core::update();
		}
	}
	tz::core::terminate();
}