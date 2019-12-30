//
// Created by Harrand on 13/12/2019.
//

#include "core/core.hpp"
#include "core/debug/assert.hpp"
#include "core/debug/print.hpp"
#include "core/tz_glad/glad_context.hpp"
#include "GLFW/glfw3.h"

namespace tz::core
{
	TopazCore::TopazCore() noexcept: tz_window(nullptr), initialised(false), secondary_windows() {}

	void TopazCore::initialise(const char* app_name)
	{
		topaz_assert(!this->initialised, "TopazCore::initialise(): Attempt to initialise but we're already marked as initialised!");
		this->initialised = true;
		// Initialise GLFW...
		tz::ext::glfw::initialise(tz::ext::glfw::WindowCreationArgs{app_name, 1920, 1080});
		// Create the GLFW window and set this to be the global GLFW context window.
		this->tz_window = std::make_unique<GLFWWindow>(tz::ext::glfw::get());
		// Give the context to GLAD and perform a procedure load of all opengl functions.
		// This overload will use the current global GLFW context (Which we *just* made).
		tz::ext::glad::load_opengl(tz::ext::glfw::get());

		tz::debug_printf("tz::initialise(): Success\n");
	}

	void TopazCore::terminate()
	{
		topaz_assert(this->initialised, "TopazCore::terminate(): Attempt to terminate but we're not marked as initialised!");
		this->initialised = false;
		// TODO: Burn stuff here.
		tz::ext::glfw::terminate();

		tz::debug_printf("tz::terminate(): Success\n");
	}

	bool TopazCore::is_initialised() const
	{
		return this->initialised;
	}

	const tz::ext::glfw::GLFWContext& TopazCore::context() const
	{
		return tz::ext::glfw::get();
	}
	
	const tz::core::IWindow& TopazCore::window() const
	{
		topaz_assert(this->tz_window != nullptr, "TopazCore::window(): Window wasn't initialised yet. Did you forget to tz::initialise?");
		return *this->tz_window;
	}
	
	tz::core::IWindow& TopazCore::window()
	{
		topaz_assert(this->tz_window != nullptr, "TopazCore::window(): Window wasn't initialised yet. Did you forget to tz::initialise?");
		return *this->tz_window;
	}
	
	const tz::core::IWindow* TopazCore::get_extra_window(std::size_t window_id) const
	{
		topaz_assert(tz::core::get().is_initialised(), "TopazCore::get_extra_window(", window_id, "): tz::core is not yet initalised!");
		if(this->secondary_windows.size() > window_id)
		{
			return this->secondary_windows[window_id].get();
		}
		return nullptr;
	}
	
	tz::core::IWindow* TopazCore::get_extra_window(std::size_t window_id)
	{
		topaz_assert(tz::core::get().is_initialised(), "TopazCore::get_extra_window(", window_id, "): tz::core is not yet initalised!");
		if(this->secondary_windows.size() > window_id)
		{
			return this->secondary_windows[window_id].get();
		}
		return nullptr;
	}
	
	void TopazCore::destroy_extra_window(std::size_t window_id)
	{
		topaz_assert(tz::core::get().is_initialised(), "TopazCore::destroy_extra_window(", window_id, "): tz::core is not yet initalised!");
		if(this->secondary_windows.size() > window_id)
		{
			this->secondary_windows[window_id] = nullptr;
		}
	}

	static TopazCore global_core;
	static ResourceManager root_manager{tz::core::project_directory};

	void initialise(const char* app_name)
	{
		global_core.initialise(app_name);
	}

	void update()
	{
		glfwPollEvents();
	}
	
	void terminate()
	{
		global_core.terminate();
	}

	TopazCore& get()
	{
		topaz_assert(global_core.is_initialised(), "Attempted tz::core::get() but tz::initialise() has not yet been invoked!");
		return global_core;
	}

	const ResourceManager& res()
	{
		return root_manager;
	}
}

