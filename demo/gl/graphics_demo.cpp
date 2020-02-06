#include "core/core.hpp"
#include "core/debug/print.hpp"
#include "geo/matrix_transform.hpp"
#include "gl/tz_stb_image/image_reader.hpp"
#include "gl/shader.hpp"
#include "gl/shader_compiler.hpp"
#include "gl/manager.hpp"
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
	tz::core::initialise("Topaz Graphics Demo");
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

		tz::gl::Mesh triangle;
		tz::gl::Mesh square;
		{
			// triangle
			{
				tz::gl::MeshData data;
				data.vertices.push_back(tz::gl::Vertex{{{-0.5f, -0.5f, 0.0f}}, {{0.0f, 0.0f}}, {{}}, {{}}, {{}}});
				data.vertices.push_back(tz::gl::Vertex{{{0.5f, -0.5f, 0.0f}}, {{1.0f, 0.0f}}, {{}}, {{}}, {{}}});
				data.vertices.push_back(tz::gl::Vertex{{{0.0f, 0.5f, 0.0f}}, {{0.5f, 1.0f}}, {{}}, {{}}, {{}}});

				tz::gl::MeshIndices indices{{0, 1, 2}};
				tz::gl::StandardDataRegionNames names
				{
					"positiont",
					"texcoordt",
					"normalt",
					"tangentt",
					"bitangentt"
				};
				triangle = {data, indices, names, "indices"};
			}

			// square
			{
				tz::gl::MeshData data;
				data.vertices.push_back(tz::gl::Vertex{{{-0.5f, -0.5f, 0.0f}}, {{0.0f, 0.0f}}, {{}}, {{}}, {{}}});
				data.vertices.push_back(tz::gl::Vertex{{{0.5f, -0.5f, 0.0f}}, {{1.0f, 0.0f}}, {{}}, {{}}, {{}}});
				data.vertices.push_back(tz::gl::Vertex{{{0.5f, 0.5f, 0.0f}}, {{1.0f, 0.5f}}, {{}}, {{}}, {{}}});
				
				data.vertices.push_back(tz::gl::Vertex{{{-0.5f, -0.5f, 0.0f}}, {{0.0f, 0.0f}}, {{}}, {{}}, {{}}});
				data.vertices.push_back(tz::gl::Vertex{{{0.5f, 0.5f, 0.0f}}, {{1.0f, 0.5f}}, {{}}, {{}}, {{}}});
				data.vertices.push_back(tz::gl::Vertex{{{-0.5f, 0.5f, 0.0f}}, {{0.0f, 0.5f}}, {{}}, {{}}, {{}}});

				tz::gl::MeshIndices indices{{0, 1, 2, 3, 4, 5}};
				tz::gl::StandardDataRegionNames names
				{
					"positions",
					"texcoords",
					"normals",
					"tangents",
					"bitangents",
				};
				square = {data, indices, names, "indices"};
			}
		}

		auto rgba_checkerboard = tz::ext::stb::read_image<tz::gl::PixelRGB8>("res/textures/bricks.jpg");
		tz::gl::Texture checkerboard;
		checkerboard.set_parameters(tz::gl::default_texture_params);
		checkerboard.set_data(rgba_checkerboard);

		prg.attach_texture(0, &checkerboard, "checkerboard");

		m.add_mesh(tz::gl::Data::Static, tz::gl::Indices::Static, triangle);
		m.add_mesh(tz::gl::Data::Static, tz::gl::Indices::Static, square);
		// so the static data stream looks something like this:
		// <t r i a n g l e  d a t a  3  v e r t i c e s> <s q u a r e  d a t a  4  v e r t i c e s       >
		// |--------|---------|-------|--------|---------| |--------|---------|-------|--------|----------|
		// positiont texcoordt normalt tangentt bitangantt positions texcoords normals tangents bitangents
		// static index stream looks like this:
		// 0	1	2	0	1	2	3	4	5
		// |--------|   |-------------------|
		//	triangle 			square
		// as we don't offset new indices, this lets us know that index 0 will start from here for square.
		m.attrib(tz::gl::Data::Static, "positions", tz::gl::fmt::three_floats);
		m.attrib(tz::gl::Data::Static, "texcoords", tz::gl::fmt::two_floats);
		// if we want to draw the triangle, we do:
		//m.attrib(tz::gl::Data::Static, "positiont", tz::gl::fmt::three_floats);
		//m.attrib(tz::gl::Data::Static, "texcoordt", tz::gl::fmt::two_floats);
		// Note: If we tried to use normals but not texcoords this would go bad.
		// We're getting lucky here because position and texcoord are the first two element regions in order.
		// We should really, really be using all components here (or not adding them at all?).

		tz::Vec3 triangle_pos{{0.0f, 0.0f, 0.0f}};
		auto add_pos = [&triangle_pos](float x, float y, float z)
		{
			triangle_pos[0] += x;
			triangle_pos[1] += y;
			triangle_pos[2] += z;
		};

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

		glClearColor(0.0f, 0.3f, 0.15f, 1.0f);
		// remember our device. it needs to know about the index snippet we want to use.
		tz::render::Device dev{wnd.get_frame(), &prg, &o};
		// well we use the same IBO for everything so this need never change:
		dev.set_handle(m.get_indices());
		// check the index stream above, this should clarify these magic numbers.
		tz::render::IndexSnippet triangle_indices{m.get_indices()};
		triangle_indices.emplace_range(0, 2);
		tz::render::IndexSnippet square_indices{m.get_indices()};
		square_indices.emplace_range(3, 8);
		// note that above we're using the square attribs and not the triangle. so we should use the square's indices too!
		dev.set_snippet(square_indices);
		// if we want to draw *both*, as it stands we will need to swap between enabling the vertex attrib arrays every time in gl::Object by exposing a new api method.
		// maybe get a better abstraction around it too because while this is a really good start, MDI will become really brain-hurty having to swap these attribs.
		// note that storing these sets of attribs and index snippets for any mesh should probably be exactly what tz::render::Object is meant to do when it's implemented!
		// that way a tz::render::Object essentially represents a mesh which can be rendered by itself even though its data may be buried in a super massive VBO somewhere! pretty nifty.
		// now this should draw without issue.
		while(!wnd.is_close_requested())
		{
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