//
// Created by Harrand on 13/12/2019.
//

#ifndef TOPAZ_OPENGL_LOADER_HPP
#define TOPAZ_OPENGL_LOADER_HPP
#include "glad/glad.h"
#include <vector>

// Forward declares
namespace tz::ext::glfw
{
	class GLFWContext;
}

namespace tz::ext::glad
{
	struct OpenGLExtension
	{
		std::string name;
	};

	/**
	 * Wrapper for interacting with GLAD.
	 * GLAD is a runtime generator-loader of OpenGL functions.
	 * It is impossible to invoke any OpenGL functions without first loading them via this context.
	 */
	class GLADContext
	{
	public:
		/**
		 * Construct a GLAD context based off of the currently active GLFW context.
		 * The context need not be loaded at this point.
		 * It is not recommended that you construct GLAD contexts manually -- tz::core should handle this for you.
		 */
		GLADContext() noexcept;
		void pre_init();
		/**
		 * Instruct GLAD to load all necessary OpenGL procedures.
		 * The GLFW context provided at construction must be initialised, or this function will assert.
		 * Invoking load() on the same context more than once will yield an assert.
		 */
		void load();
		/**
		 * Query as to whether this GLAD context has yet been instructed to load OpenGL procedures.
		 * @return - True if OpenGL procedures have been loaded, otherwise false
		 */
		bool is_loaded() const;
		bool supports_extension(const char* name) const;
		std::size_t extensions_count() const;
		const OpenGLExtension& get_extension(std::size_t extension_id) const;
		friend void load_opengl();
	private:
		void populate_extensions();
		/// GLFW context dependency.
		const tz::ext::glfw::GLFWContext* glfw_context;
		/// Have we tried to load yet?
		bool loaded;
		std::vector<OpenGLExtension> supported_extensions;
	};
	
	static GLADContext global_context;
	
	/**
	 * Load OpenGL using the currently active GLFW context.
	 * tz::core::initialise() invokes this, and invoking this more than once will assert.
	 * It is not recommended that you invoke this function unless you know what you're doing.
	 */
	void load_opengl();
	/**
	 * TODO: Document
	 */
	void custom_error(GLenum severity, const GLchar *message);
	/**
	 * TODO: Document
	 */
	std::pair<int, int> gl_version();
	/**
	 * TODO: Document
	 */
	GLADContext& get();
}

#endif //TOPAZ_OPENGL_LOADER_HPP
