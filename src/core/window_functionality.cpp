#include "core/window_functionality.hpp"
#include "core/assert.hpp"
#include "core/peripherals/keyboard.hpp"
#include "core/peripherals/mouse.hpp"
#include "core/profiling/zone.hpp"

#if TZ_OGL
#include "glad/glad.h"
#endif

namespace tz
{
	WindowFunctionality::WindowFunctionality(GLFWwindow* wnd):
	wnd(wnd),
	window_resize_callbacks(),
	kb_state(),
	mb_state(){}

	GLFWwindow* WindowFunctionality::get_middleware_handle() const
	{
		return this->wnd;
	}

	bool WindowFunctionality::is_close_requested() const
	{
		this->ensure();
		return glfwWindowShouldClose(this->wnd) == GLFW_TRUE;
	}

	float WindowFunctionality::get_width() const
	{
		return static_cast<float>(this->get_size().first);
	}

	float WindowFunctionality::get_height() const
	{
		return static_cast<float>(this->get_size().second);
	}

	const KeyboardState& WindowFunctionality::get_keyboard_state() const
	{
		return this->kb_state;
	}

	const MouseButtonState& WindowFunctionality::get_mouse_button_state() const
	{
		return this->mb_state;
	}

	void WindowFunctionality::update()
	{
		TZ_PROFZONE("WindowFunctionality::update", TZ_PROFCOL_YELLOW);
		glfwPollEvents();
		#if TZ_OGL
			// OpenGL only
			glfwSwapBuffers(this->wnd);
		#endif
	}

	/*
	void WindowFunctionality::set_render_target() const
	{
		#if TZ_VULKAN

		#elif TZ_OGL
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, static_cast<GLsizei>(this->get_width()), static_cast<GLsizei>(this->get_height()));
		#endif
	}
	*/

	void WindowFunctionality::block_until_event_happens()
	{
		glfwWaitEvents();
	}

	void WindowFunctionality::handle_key_event(int key, int scancode, int action, [[maybe_unused]] int mods)
	{
		KeyPressType t;
		switch(action)
		{
			case GLFW_PRESS:
			[[fallthrough]];
			case GLFW_REPEAT:
				t = KeyPressType::Press;
			break;
			case GLFW_RELEASE:
				t = KeyPressType::Release;
			break;
			default:
				return;
			break;
		}
		const char* glfw_key_name = glfwGetKeyName(key, scancode);
		if(glfw_key_name == nullptr)
		{
			// GLFW key has no name. We don't suppose those keys.
			return;
		}
		KeyInfo tz_key = peripherals::keyboard::get_key(glfw_key_name[0]);
		if(tz_key == peripherals::keyboard::key_null)
		{
			// GLFW key doesn't translate cleanly into a TZ key. We don't support such keys.
			return;
		}
		this->kb_state.update
		({
			.key = tz_key,
			.type = t
		});
	}

	void WindowFunctionality::handle_mouse_event(int button, int action, int mods)
	{
		MouseButton tz_button;
		switch(button)
		{
			case GLFW_MOUSE_BUTTON_LEFT:
				tz_button = MouseButton::Left;
			break;
			case GLFW_MOUSE_BUTTON_RIGHT:
				tz_button = MouseButton::Right;
			break;
			case GLFW_MOUSE_BUTTON_MIDDLE:
				tz_button = MouseButton::Middle;
			break;
			case GLFW_MOUSE_BUTTON_4:
				tz_button = MouseButton::Extra0;
			break;
			case GLFW_MOUSE_BUTTON_5:
				tz_button = MouseButton::Extra1;
			break;
			case GLFW_MOUSE_BUTTON_6:
				tz_button = MouseButton::Extra2;
			break;
			case GLFW_MOUSE_BUTTON_7:
				tz_button = MouseButton::Extra3;
			break;
			case GLFW_MOUSE_BUTTON_8:
				tz_button = MouseButton::Extra4;
			break;
		}
		MouseButtonInfo button_info = peripherals::mouse::get_mouse_button(tz_button);
		if(button_info == peripherals::mouse::mouse_button_null)
		{
			return;
		}
		double mxpos, mypos;
		glfwGetCursorPos(this->wnd, &mxpos, &mypos);
		this->mb_state.update(
		{
			.button = button_info,
			.press_position = {static_cast<unsigned int>(mxpos), static_cast<unsigned int>(mypos)}
		}, action == GLFW_PRESS);
	}

	std::pair<int, int> WindowFunctionality::get_size() const
	{
		int w, h;
		glfwGetFramebufferSize(this->wnd, &w, &h);
		return {w, h};
	}

	void WindowFunctionality::ensure() const
	{
		tz_assert(this->wnd != nullptr, "WindowFunctionality::ensure(): Failed");
	}
}
