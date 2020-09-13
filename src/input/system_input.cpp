//
// Created by Harrand on 25/12/2019.
//
#include "input/system_input.hpp"
#include "GLFW/glfw3.h"

namespace tz::input
{
	const char* get_clipboard_data()
	{
		// The resultant string is valid until the next set_clipboard_data or get_clipboard_data invocation.
		// The user is discouraged from caching the string for this reason.
		return glfwGetClipboardString(nullptr);
	}

	void set_clipboard_data(const char* data)
	{
		// Note: GLFW will copy the underlying string before the function returns.
		glfwSetClipboardString(nullptr, data);
	}
}