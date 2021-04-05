//
// Created by Harrand on 13/12/2019.
//

#ifndef TOPAZ_CORE_HPP
#define TOPAZ_CORE_HPP
#include "core/window.hpp"
#include "core/resource_manager.hpp"
#include "core/settings.hpp"
#include <memory>

/*! \mainpage Topaz v2.0-indev
 *
 * \section intro_sec Introduction
 *
 * Welcome to the Topaz documentation. This was generated automatically via https://github.com/Harrand/Topaz/actions.
 * 
 * If you're here to learn how to use Topaz, it is also recommended to visit the wiki which contains examples and explanations of various engine features. You can find it at: https://github.com/Harrand/Topaz/wiki.
 *
 * \section modules_sec Topaz Modules
 * The Topaz engine is comprised of several large modules. Each have their own responsibility, namespace and location with the source code. For more detailed information about the modules, visit the 'Modules' tab above. A summary of the modules has been provided for you below.
 *
 * \subsection tzcore Topaz Core
 *
 * The core module of Topaz.
 * Source location: Topaz/src/core
 * Namespace: `tz`
 * 
 * \subsection tzalgo Topaz Algorithms
 * 
 * Contains various common algorithms within engine code and is also available to game code.
 * Source location: Topaz/src/algo
 * Namespace: `tz::algo`
 * 
 * \subsection tzdui Topaz Debug UI
 * Using Imgui as a backend, display ui within any Topaz application used for debugging purposes. This was initially designed to be debug-only, but exists in all build configs.
 * Source location: Topaz/src/dui
 * Namespace: `tz::dui`
 * 
 * \subsection tzext Topaz External Code
 * Consists of wrappers for external libraries used by Topaz.
 * Source location: Topaz/src/ext
 * Namespace: `tz::ext`
 * 
 * \subsection tzgeo Topaz Geometry
 * Consists of basic geometric structures to represent various shapes within 3D space.
 * Source location: Topaz/src/geo
 * Namespace: `tz::geo`
 * 
 * \subsection tzgl Topaz Graphics Library
 * Low-level 3D graphics functionality. Currently always uses OpenGL as its underlying API, although Vulkan support is planned.
 * Source location: Topaz/src/gl
 * Namespace: `tz::gl`
 * 
 * \subsection tzinput Topaz Input
 * Input events, callbacks and device support.
 * Source location: Topaz/src/input
 * Namespace: `tz::input`
 * 
 * \subsection tzmemory Topaz Memory
 * Helper structures and functions used for memory management. Mainly intended for engine-side code but will be available to game code aswell.
 * Source location: Topaz/src/memory
 * Namespace: `tz::mem`
 * 
 * \subsection tzphys Topaz Physics
 * Classes, logic and functions for physics simulations within a 3D/2D world.
 * Source location: Topaz/src/phys
 * Namespace: `tz::phys`
 * 
 * \subsection tzrender Topaz Render
 * High-level 3D graphics functionality. Built directly ontop of `tz::gl`.
 * Source location: Topaz/src/render
 * Namespace: `tz::render`
 * 
 */

// Forward declares
namespace tz::ext
{
	namespace glfw
	{
		class GLFWContext;
	}

	namespace glad
	{
		class GLADContext;
	}
}

namespace tz
{
	/**
	 * \addtogroup tz_core Topaz Core Library (tz)
	 * A collection of platform-agnostic core interfaces.
	 * @{
	 */

	/**
	 * Wrapper class responsible for handling initialisation and termination of essential external modules.
	 * Examples of such modules include GLFW and GLAD.
	 * tz::initialise() handles this for you. Don't mess with these unless you know what you're doing!
	 */
	class TopazCore
	{
	public:
		/**
		 * Constructs an empty core. Doesn't achieve anything until initialisation.
		 */
		TopazCore() noexcept;
		~TopazCore() = default;
		/**
		 * Initialise Topaz with the given name. This will be used as the initial title for the window.
		 * @param app_name C-string corresponding to the name of the application
		 * @param visible Whether the window should initially be visible.
		 */
		void initialise(const char* app_name, bool visible);
		/**
		 * Terminate Topaz. This must be invoked before runtime ends to prevent various memory leaks.
		 */
		void terminate();
		/**
		 * Query as to whether this core has been initialised recently.
		 * Note: If initialise(...) is called, followed by terminate(), the core is considered uninitialised.
		 * @return True if the core is initialised, otherwise false
		 */
		bool is_initialised() const;
		/**
		 * Retrieve the main context provided by topaz.
		 * @return Reference to the initial context
		 */
		const tz::ext::glfw::GLFWContext& context() const;
		const tz::ext::glad::GLADContext& glad_context() const;
		/**
		 * Retrieve the window attached to the main context provided by topaz.
		 * @return Reference to the initial context window
		 */
		const IWindow& window() const;
		/**
		 * Retrieve the window attached to the main context provided by topaz.
		 * @return Reference to the initial context window.
		 */
		IWindow& window();
		/**
		 * Retrieve the current render settings. Settings can be changed here.
		 * @return Reference to the current render settings.
		 */
		tz::RenderSettings& render_settings();
	private:
		/// High-level window is stored here. Won't store a value until we initialise!
		IWindow* tz_window;
		/// Have we been initialised?
		bool initialised;
		/// Render settings.
		tz::RenderSettings current_render_settings;
	};

	struct invisible_tag_t{};

	constexpr invisible_tag_t invisible_tag{};

	/**
	 * Instruct topaz to initialise all core modules.
	 * This will provide you with a window, ready to be rendered into and receive input.
	 * After this, you can invoke get() to retrieve the core instance.
	 * @param app_name Name of the application; will be the title of the window too
	 */
	void initialise(const char* app_name);
	void initialise(const char* app_name, invisible_tag_t t);
	/**
	 * Advance topaz. This will poll all window events for the main core.
	 * You should invoke this every frame.
	 */
	void update();
	/**
	 * Terminate topaz. Ensure that all core modules are safely terminated in the correct order.
	 */
	void terminate();
	/**
	 * Retrieve an instance of the topaz core.
	 * @return Reference to the topaz core
	 */
	TopazCore& get();
	/**
	 * Retrieve the root resource manager, which targets the project's root directory. If this header is within "Topaz/src/core", then the root directory is "Topaz".
	 */
	const ResourceManager& res();

	/**
	 * @}
	 */
	constexpr char project_directory[] = TOPAZ_PROJECT_DIR;
}
#endif //TOPAZ_CORE_HPP
