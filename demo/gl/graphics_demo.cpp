#include "core/core.hpp"
#include "gl/shader.hpp"
#include "gl/shader_compiler.hpp"
#include "gl/object.hpp"
#include "gl/buffer.hpp"

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n\0";

int main()
{
	// Minimalist Graphics Demo.
	tz::core::initialise("Topaz Graphics Demo");
	{
		tz::gl::ShaderCompiler cpl;
		tz::gl::ShaderProgram prg;
		tz::gl::Shader* vs = prg.emplace(tz::gl::ShaderType::Vertex);
		vs->upload_source(vertexShaderSource);
		tz::gl::Shader* fs = prg.emplace(tz::gl::ShaderType::Fragment);
		fs->upload_source(fragmentShaderSource);

		cpl.compile(*vs);
		cpl.compile(*fs);
		cpl.link(prg);

		float vertices[] = {
        -0.5f, -0.5f, 0.0f, // left  
         0.5f, -0.5f, 0.0f, // right 
         0.0f,  0.5f, 0.0f  // top   
    	};
		tz::gl::Object o;
		std::size_t vbo_id = o.emplace_buffer<tz::gl::BufferType::Array>();
		tz::gl::VBO* vbo = o.get<tz::gl::BufferType::Array>(vbo_id);

		vbo->bind();
		vbo->resize(sizeof(vertices));
		vbo->send(vertices);

		o.format(vbo_id, tz::gl::fmt::three_floats);

		vbo->unbind();
		o.unbind();

		tz::core::IWindow& wnd = tz::core::get().window();
		while(!wnd.is_close_requested())
		{
			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        	glClear(GL_COLOR_BUFFER_BIT);
			prg.bind();
			o.bind();
			glDrawArrays(GL_TRIANGLES, 0, 3);

			wnd.update();
			tz::core::update();
		}
	}
	tz::core::terminate();
}