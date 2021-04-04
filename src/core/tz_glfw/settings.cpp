#include "core/tz_glfw/settings.hpp"
#include "core/debug/assert.hpp"
#include "GLFW/glfw3.h"

namespace tz::ext::glfw
{
    std::pair<int, int> get_monitor_dimensions()
	{
		//check_one_monitor();
		GLFWmonitor* default_monitor = glfwGetPrimaryMonitor();
		topaz_assert(default_monitor != nullptr, "glfwGetPrimaryMonitor() returned nullptr");
		const GLFWvidmode* vm = glfwGetVideoMode(default_monitor);
		return {vm->width, vm->height};
	}

	int get_monitor_width()
	{
		return get_monitor_dimensions().first;
	}

	int get_monitor_height()
	{
		return get_monitor_dimensions().second;
	}
}