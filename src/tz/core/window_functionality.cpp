#include "tz/core/window_functionality.hpp"
#include "hdk/debug.hpp"
#include "tz/core/peripherals/keyboard.hpp"
#include "tz/core/peripherals/mouse.hpp"
#include "tz/core/profiling/zone.hpp"

#include "tz/dbgui/dbgui.hpp"

#include "GLFW/glfw3.h"
#if TZ_OGL
#include "glad/glad.h"
#if HDK_PROFILE
#include "TracyOpenGL.hpp"
#endif // HDK_PROFILE
#endif

namespace tz
{
	tz::KeyInfo get_key_from_glfw_code(int glfw_key, int scancode);

	WindowFunctionality::WindowFunctionality(GLFWwindow* wnd):
	wnd(wnd),
	window_resize_callbacks(),
	window_move_callbacks(),
	kb_state(),
	mb_state(),
	mp_state(){}

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

	void WindowFunctionality::set_width(float width)
	{
		this->set_size(width, this->get_height());
	}

	void WindowFunctionality::set_height(float height)
	{
		this->set_size(this->get_width(), height);
	}

	bool WindowFunctionality::is_minimised() const
	{
		return glfwGetWindowAttrib(this->wnd, GLFW_ICONIFIED);
	}

	bool WindowFunctionality::is_maximised() const
	{
		return glfwGetWindowAttrib(this->wnd, GLFW_MAXIMIZED);
	}

	bool WindowFunctionality::is_focused() const
	{
		return glfwGetWindowAttrib(this->wnd, GLFW_FOCUSED);
	}

	bool WindowFunctionality::is_resizeable() const
	{
		return glfwGetWindowAttrib(this->wnd, GLFW_RESIZABLE);
	}

	WindowFunctionality::ResizeCallbackType& WindowFunctionality::on_resize()
	{
		return this->window_resize_callbacks;
	}

	WindowFunctionality::MoveCallbackType& WindowFunctionality::on_move()
	{
		return this->window_move_callbacks;
	}

	const KeyboardState& WindowFunctionality::get_keyboard_state() const
	{
		return this->kb_state;
	}

	const MouseButtonState& WindowFunctionality::get_mouse_button_state() const
	{
		return this->mb_state;
	}

	const MousePositionState& WindowFunctionality::get_mouse_position_state() const
	{
		return this->mp_state;
	}

	void WindowFunctionality::update()
	{
		TZ_PROFZONE("WindowFunctionality::update", TZ_PROFCOL_YELLOW);
		glfwPollEvents();
		{
			double mx, my;
			glfwGetCursorPos(this->wnd, &mx, &my);
			this->mp_state.update({static_cast<float>(mx), static_cast<float>(my)});
		}
		#if TZ_VULKAN
			// If we're on vulkan and we're minimised, halt until otherwise.
			while(this->get_width() == 0 || this->get_height() == 0)
			{
				this->block_until_event_happens();
			}
		#elif TZ_OGL
			// OpenGL only
			#if HDK_PROFILE
				TracyGpuCollect;
			#endif // HDK_PROFILE
		#endif
	}

	void WindowFunctionality::begin_frame()
	{
		TZ_FRAME_BEGIN;
		this->update();
		tz::dbgui::begin_frame();
	}

	void WindowFunctionality::end_frame()
	{
		tz::dbgui::end_frame();
		TZ_FRAME_END;
	}

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
		KeyInfo tz_key = get_key_from_glfw_code(key, scancode);
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

	void WindowFunctionality::handle_mouse_event(int button, int action, [[maybe_unused]] int mods)
	{
		MouseButton tz_button;
		switch(button)
		{
			default:
				hdk::error("Unrecognised MouseButton. Memory corruption? Please submit a bug report.");
			[[fallthrough]];
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
			.press_position = this->mp_state.get_mouse_position()
		}, action == GLFW_PRESS);
	}

	void WindowFunctionality::handle_scroll_event(float xoffset, float yoffset)
	{
		this->mb_state.update_scroll({xoffset, yoffset});
	}

	std::pair<int, int> WindowFunctionality::get_size() const
	{
		int w, h;
		glfwGetFramebufferSize(this->wnd, &w, &h);
		return {w, h};
	}

	void WindowFunctionality::set_size(int w, int h)
	{
		glfwSetWindowSize(this->wnd, w, h);
	}

	void WindowFunctionality::ensure() const
	{
		hdk::assert(this->wnd != nullptr, "WindowFunctionality::ensure(): Failed");
	}

	tz::KeyInfo get_key_from_glfw_code(int glfw_key, int scancode)
	{
		tz::KeyCode kc;
		switch(glfw_key)
		{
			case GLFW_KEY_SPACE:
				kc = tz::KeyCode::Space;
			break;
			case GLFW_KEY_LEFT_CONTROL:
				kc = tz::KeyCode::LeftControl;
			break;
			case GLFW_KEY_LEFT_SHIFT:
				kc = tz::KeyCode::LeftShift;
			break;
			case GLFW_KEY_LEFT_ALT:
				kc = tz::KeyCode::LeftAlt;
			break;
			case GLFW_KEY_RIGHT_ALT:
				kc = tz::KeyCode::AltGr;
			break;
			case GLFW_KEY_RIGHT_CONTROL:
				kc = tz::KeyCode::RightControl;
			break;
			case GLFW_KEY_RIGHT_SHIFT:
				kc = tz::KeyCode::RightShift;
			break;
			case GLFW_KEY_ENTER:
				kc = tz::KeyCode::Enter;
			break;
			case GLFW_KEY_BACKSPACE:
				kc = tz::KeyCode::Backspace;
			break;
			case GLFW_KEY_UP:
				kc = tz::KeyCode::ArrowUp;
			break;
			case GLFW_KEY_LEFT:
				kc = tz::KeyCode::ArrowLeft;
			break;
			case GLFW_KEY_DOWN:
				kc = tz::KeyCode::ArrowDown;
			break;
			case GLFW_KEY_RIGHT:
				kc = tz::KeyCode::ArrowRight;
			break;
			case GLFW_KEY_CAPS_LOCK:
				kc = tz::KeyCode::CapsLock;
			break;
			case GLFW_KEY_TAB:
				kc = tz::KeyCode::Tab;
			break;
			case GLFW_KEY_ESCAPE:
				kc = tz::KeyCode::Escape;
			break;
			default:
				// Try and retrieve by key name. This should catch all printable keys.
				const char* glfw_key_name = glfwGetKeyName(glfw_key, scancode);
				if(glfw_key_name == nullptr || glfw_key_name[0] == '\0')
				{
					return peripherals::keyboard::key_null;
				}
				return peripherals::keyboard::get_key(glfw_key_name[0]);
			break;
		}
		return peripherals::keyboard::get_key(kc);
	}
}
