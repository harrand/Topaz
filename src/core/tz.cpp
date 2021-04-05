//
// Created by Harrand on 13/12/2019.
//

#include "core/tz.hpp"
#include "core/debug/assert.hpp"
#include "core/debug/print.hpp"
#include "ext/tz_glad/glad_context.hpp"
#include "ext/tz_glfw/interface.hpp"
#include "dui/window.hpp"
#include "GLFW/glfw3.h"

namespace tz
{
	TopazCore::TopazCore() noexcept: tz_window(nullptr), initialised(false){}

	void TopazCore::initialise(const char* app_name, bool visible)
	{
		topaz_assert(!this->initialised, "TopazCore::initialise(): Attempt to initialise but we're already marked as initialised!");
		this->initialised = true;
		// Initialise GLFW...
		// TODO: Deal with visibility hint
		tz::ext::glfw::WindowHintCollection hints;
		hints.add(GLFW_VISIBLE, visible ? GL_TRUE : GL_FALSE);
		glfwSetErrorCallback(tz::ext::glfw::default_error_callback);
		tz::ext::glfw::initialise(tz::ext::glfw::WindowCreationArgs{app_name, 1920, 1080}, hints);
		// Setup a debug callback
		tz::ext::glfw::get().set_error_callback(tz::ext::glfw::default_error_callback);
		// Create the GLFW window and set this to be the global GLFW context window.
		this->tz_window = &tz::ext::glfw::get().get_window();
		this->tz_window->set_active_context();
		tz::ext::glad::get().load();
		tz::dui::initialise();
	}

	void TopazCore::terminate()
	{
		topaz_assert(this->initialised, "TopazCore::terminate(): Attempt to terminate but we're not marked as initialised!");
		this->initialised = false;
		tz::dui::terminate();
		// TODO: Burn stuff here.
		tz::ext::glfw::terminate();
	}

	bool TopazCore::is_initialised() const
	{
		return this->initialised;
	}

	const tz::ext::glad::GLADContext& TopazCore::glad_context() const
	{
		return tz::ext::glad::get();
	}
	
	const tz::IWindow& TopazCore::window() const
	{
		topaz_assert(this->tz_window != nullptr, "TopazCore::window(): Window wasn't initialised yet. Did you forget to tz::initialise?");
		return *this->tz_window;
	}
	
	tz::IWindow& TopazCore::window()
	{
		topaz_assert(this->tz_window != nullptr, "TopazCore::window(): Window wasn't initialised yet. Did you forget to tz::initialise?");
		return *this->tz_window;
	}

	tz::RenderSettings& TopazCore::render_settings()
	{
		return this->current_render_settings;
	}

	static TopazCore global_core;
	static ResourceManager root_manager{tz::project_directory};

	void initialise(const char* app_name)
	{
		global_core.initialise(app_name, true);
	}

	void initialise(const char* app_name, [[maybe_unused]] invisible_tag_t t)
	{
		global_core.initialise(app_name, false);
	}

	void update()
	{
		glfwPollEvents();
		tz::dui::update();
	}
	
	void terminate()
	{
		global_core.terminate();
	}

	TopazCore& get()
	{
		topaz_assert(global_core.is_initialised(), "Attempted tz::get() but tz::initialise() has not yet been invoked!");
		return global_core;
	}

	const ResourceManager& res()
	{
		return root_manager;
	}
}

