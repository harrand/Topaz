//
// Created by Harrand on 13/12/2019.
//

#ifndef TOPAZ_CORE_HPP
#define TOPAZ_CORE_HPP
#include "core/window.hpp"
#include "core/resource_manager.hpp"
#include <memory>

/*! \mainpage Topaz 2
 *
 * \section intro_sec Introduction
 *
 * This is the introduction.
 *
 * \section install_sec Installation
 *
 * \subsection step1 Step 1: Opening the box
 *
 * etc...
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

namespace tz::core
{
	/**
	 * \addtogroup tz_core Topaz Core Library (tz::core)
	 * A collection of platform-agnostic core interfaces.
	 * @{
	 */

	/**
	 * Wrapper class responsible for handling initialisation and termination of essential external modules.
	 * Examples of such modules include GLFW and GLAD.
	 * tz::core::initialise() handles this for you. Don't mess with these unless you know what you're doing!
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
		 */
		void initialise(const char* app_name);
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
		 * @return Reference to the initial context window
		 */
		IWindow& window();
		void enable_wireframe_mode(bool wireframe) const;
		/**
		 * TODO: Document
		 * @tparam WindowType
		 * @tparam Args
		 * @param args
		 * @return
		 */
		template<typename WindowType, typename... Args>
		std::size_t emplace_extra_window(Args&&... args);
		const IWindow* get_extra_window(std::size_t window_id) const;
		IWindow* get_extra_window(std::size_t window_id);
		void destroy_extra_window(std::size_t window_id);
	private:
		/// High-level window is stored here. Won't store a value until we initialise!
		std::unique_ptr<IWindow> tz_window;
		/// The core can store any number of secondary windows.
		std::vector<std::unique_ptr<IWindow>> secondary_windows;
		/// Have we been initialised?
		bool initialised;
	};

	/**
	 * Instruct topaz to initialise all core modules.
	 * This will provide you with a window, ready to be rendered into and receive input.
	 * After this, you can invoke get() to retrieve the core instance.
	 * @param app_name Name of the application; will be the title of the window too
	 */
	void initialise(const char* app_name);
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
	
	const ResourceManager& res();

	/**
	 * @}
	 */
	constexpr char project_directory[] = TOPAZ_PROJECT_DIR;
}

#include "core/core.inl"
#endif //TOPAZ_CORE_HPP
