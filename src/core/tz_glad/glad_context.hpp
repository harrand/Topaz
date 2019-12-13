//
// Created by Harrand on 13/12/2019.
//

#ifndef TOPAZ_OPENGL_LOADER_HPP
#define TOPAZ_OPENGL_LOADER_HPP
#include "glad/glad.h"

// Forward declares
namespace tz::ext::glfw
{
	class GLFWContext;
}

namespace tz::ext::glad
{
	class GLADContext
	{
	public:
		explicit GLADContext(const tz::ext::glfw::GLFWContext& glfw_context) noexcept;
		GLADContext() noexcept;
		void load();
		bool is_loaded() const;
	private:
		const tz::ext::glfw::GLFWContext* glfw_context;
		bool loaded;
	};
	
	static GLADContext global_context;
	
	void load_opengl();
	void load_opengl(const tz::ext::glfw::GLFWContext& specific_context);
}

#endif //TOPAZ_OPENGL_LOADER_HPP
