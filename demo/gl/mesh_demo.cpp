#include "core/core.hpp"
#include "core/debug/print.hpp"
#include "geo/matrix_transform.hpp"
#include "gl/tz_stb_image/image_reader.hpp"
#include "gl/shader.hpp"
#include "gl/shader_compiler.hpp"
#include "gl/manager.hpp"
#include "gl/mesh_loader.hpp"
#include "gl/buffer.hpp"
#include "gl/frame.hpp"
#include "gl/modules/ubo.hpp"
#include "gl/texture.hpp"
#include "render/device.hpp"
#include "GLFW/glfw3.h"

const char *vertexShaderSource = "#version 430\n"
    "layout (location = 0) in vec3 aPos;\n"
	"layout (location = 1) in vec2 aTexcoord;\n"
	"#ubo matrices\n"
	"{\n"
	"	mat4 mvp;\n"
	"};\n"
	"out vec2 texcoord;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = mvp * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
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
	tz::core::initialise("Topaz Mesh Demo");
	{
		tz::gl::Manager m;
        tz::gl::Object& o = *m;
		tz::gl::p::UBOModule* ubo_module = nullptr;
		tz::gl::ShaderPreprocessor pre{vertexShaderSource};
		{
			std::size_t ubo_module_id = pre.emplace_module<tz::gl::p::UBOModule>(&o);
			pre.preprocess();
			ubo_module = static_cast<tz::gl::p::UBOModule*>(pre[ubo_module_id]);
		}
		std::size_t ubo_id = ubo_module->get_buffer_id(ubo_module->size() - 1);
		tz::gl::UBO* ubo = o.get<tz::gl::BufferType::UniformStorage>(ubo_id);
		ubo->terminal_resize(sizeof(tz::Mat4) * 3);
		tz::mem::UniformPool<tz::Mat4> matrix = ubo->map_pool<tz::Mat4>();

		tz::gl::ShaderCompiler cpl;
		tz::gl::ShaderProgram prg;
		tz::gl::Shader* vs = prg.emplace(tz::gl::ShaderType::Vertex);
		vs->upload_source(pre.result());
		tz::gl::Shader* fs = prg.emplace(tz::gl::ShaderType::Fragment);
		fs->upload_source(fragmentShaderSource);

		cpl.compile(*vs);
		cpl.compile(*fs);
		cpl.link(prg);

        tz::gl::IndexedMesh triangle;
        triangle.vertices.push_back(tz::gl::Vertex{{{-0.5f, 0.5f, 0.0f}}, {{0.0f, 0.0f}}, {{}}, {{}}, {{}}});
        triangle.vertices.push_back(tz::gl::Vertex{{{0.5f, 0.5f, 0.0f}}, {{1.0f, 0.0f}}, {{}}, {{}}, {{}}});
        triangle.vertices.push_back(tz::gl::Vertex{{{0.0f, 1.5f, 0.0f}}, {{0.5f, 1.0f}}, {{}}, {{}}, {{}}});
        triangle.indices = {0, 1, 2};

        tz::gl::IndexedMesh square;
        square.vertices.push_back(tz::gl::Vertex{{{-0.5f, -0.5f, 0.0f}}, {{0.0f, 0.0f}}, {{}}, {{}}, {{}}});
        square.vertices.push_back(tz::gl::Vertex{{{0.5f, -0.5f, 0.0f}}, {{1.0f, 0.0f}}, {{}}, {{}}, {{}}});
        square.vertices.push_back(tz::gl::Vertex{{{0.5f, 0.5f, 0.0f}}, {{1.0f, 0.5f}}, {{}}, {{}}, {{}}});
        
        square.vertices.push_back(tz::gl::Vertex{{{-0.5f, -0.5f, 0.0f}}, {{0.0f, 0.0f}}, {{}}, {{}}, {{}}});
        square.vertices.push_back(tz::gl::Vertex{{{0.5f, 0.5f, 0.0f}}, {{1.0f, 0.5f}}, {{}}, {{}}, {{}}});
        square.vertices.push_back(tz::gl::Vertex{{{-0.5f, 0.5f, 0.0f}}, {{0.0f, 0.5f}}, {{}}, {{}}, {{}}});
        square.indices = {0, 1, 2, 3, 4, 5};

		tz::gl::IndexedMesh monkey_head = tz::gl::load_mesh("res/models/monkeyhead.obj");
		tz::debug_printf("monkey head data size = %zu bytes, indices size = %zu bytes", monkey_head.data_size_bytes(), monkey_head.indices_size_bytes());

		auto rgba_checkerboard = tz::ext::stb::read_image<tz::gl::PixelRGB8>("res/textures/bricks.jpg");
		tz::gl::Texture checkerboard;
		checkerboard.set_parameters(tz::gl::default_texture_params);
		checkerboard.set_data(rgba_checkerboard);

		prg.attach_texture(0, &checkerboard, "checkerboard");

		tz::Vec3 triangle_pos{{0.0f, 0.0f, 0.0f}};
		auto add_pos = [&triangle_pos](float x, float y, float z)
		{
			triangle_pos[0] += x;
			triangle_pos[1] += y;
			triangle_pos[2] += z;
		};

		tz::gl::Manager::Handle triangle_handle = m.add_mesh(triangle);
        tz::gl::Manager::Handle square_handle = m.add_mesh(square);
		tz::gl::Manager::Handle monkeyhead_handle = m.add_mesh(monkey_head);

		float rotation_x = 0.0f;
		float rotation_y = 0.0f;

		tz::core::IWindow& wnd = tz::core::get().window();
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

		glClearColor(0.3f, 0.15f, 0.0f, 1.0f);
		tz::render::Device dev{wnd.get_frame(), &prg, &o};
		dev.set_handle(m.get_indices());

        tz::render::IndexSnippet triangle_snip{m.get_indices()};
        triangle_snip.emplace_range(0, 2);

        tz::render::IndexSnippet square_snip{m.get_indices()};
        square_snip.emplace_range(3, 8, 3);

		tz::render::IndexSnippet monkey_snip{m.get_indices()};
		constexpr std::size_t monkey_begin = 9;
		monkey_snip.emplace_range(monkey_begin, monkey_head.indices.size() - 4, monkey_begin);

        tz::render::IndexSnippet double_snip{m.get_indices()};
        double_snip.emplace_range(0, 2, m.get_indices_offset(triangle_handle)); // Triangle
        double_snip.emplace_range(3, 8, m.get_indices_offset(square_handle)); // Square
        // This should do both!

        //dev.set_snippet(triangle_snip);
        //dev.set_snippet(square_snip);
		dev.set_snippet(monkey_snip);
        //dev.set_snippet(double_snip);

		// TODO: Remove
		// Temporary FPS Counter Logic
		double last_time = glfwGetTime();
		int frames = 0;
		while(!wnd.is_close_requested())
		{
			double time = glfwGetTime();
			frames++;
			if(time - last_time >= 1.0)
			{
				std::printf("%g ms/frame\n", 1000.0/static_cast<double>(frames));
				std::printf("%d fps\n", frames);
				frames = 0;
				last_time += 1.0;
			}

			rotation_y += 0.02f;

        	dev.clear();
			o.bind();
			tz::Mat4 m = tz::geo::model(triangle_pos, tz::Vec3{{0.0f, rotation_y, 0.0f}}, tz::Vec3{{1.0f, 1.0f, 1.0f}});
			tz::Mat4 v = tz::geo::view(tz::Vec3{{0.0f, 0.0f, 5.0f}}, tz::Vec3{{0.0f, 0.0f, 0.0f}});
			tz::Mat4 p = tz::geo::perspective(1.57f, 1920.0f/1080.0f, 0.1f, 1000.0f);
			matrix.set(0, p * v * m);
			ubo->bind();
			dev.render();
			wnd.update();
			tz::core::update();
		}
	}
	tz::core::terminate();
}