#if TZ_OGL
#include "core/assert.hpp"
#include "gl/impl/backend/ogl/setup/opengl_instance.hpp"
#include "GLFW/glfw3.h"
#include <iostream>

namespace tz::gl::ogl
{
	void OpenGLInstance::opengl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, [[maybe_unused]] GLsizei length, const GLchar* message, [[maybe_unused]] const void* user_data)
	{
		// ignore non-significant error/warning codes
		if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

		std::cout << "---------------" << std::endl;
		std::cout << "Debug message (" << id << "): " <<  message << std::endl;

		bool erroneous = false;
		switch (source)
		{
			case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
			case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
			case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
			case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
			case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
		} std::cout << std::endl;

		switch (type)
		{
			case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; erroneous = true; break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; erroneous = true; break; 
			case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
			case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
			case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
			case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
			case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
			case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
		} std::cout << std::endl;
		
		switch (severity)
		{
			case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
			case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
			case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
			case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
		} std::cout << std::endl;
		std::cout << std::endl;
		if(erroneous)
		{
			tz_error("GL Error Detected.");
		}
	}

	OpenGLInstance::OpenGLInstance([[maybe_unused]] tz::GameInfo game_info)
	{
		[[maybe_unused]] int glad_load_result = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
		tz_assert(glad_load_result != 0, "gladLoadGLLoader returned error");

		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLInstance::opengl_debug_callback, nullptr);
	}

	OpenGLInstance::~OpenGLInstance()
	{
		
	}
}

#endif // TZ_OGL