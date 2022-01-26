#ifndef TOPAZ_CORE_WINDOW_FUNCTIONALITY_HPP
#define TOPAZ_CORE_WINDOW_FUNCTIONALITY_HPP
#include "core/types.hpp"
#include "core/keyboard_state.hpp"
#include <vector>
#include <functional>

namespace tz
{
	/**
	 * \addtogroup tz_core Topaz Core Library (tz)
	 * A collection of platform-agnostic core interfaces.
	 * @{
	 */
	using WindowResizeSignature = void(int width, int height);
	template<typename F>
	concept WindowResizeConcept = tz::Action<F, int, int>;

	class WindowFunctionality
	{
	public:
		WindowFunctionality(GLFWwindow* wnd);
		GLFWwindow* get_middleware_handle() const;
		bool is_close_requested() const;
		
		void add_resize_callback(WindowResizeConcept auto on_window_resize);
		float get_width() const;
		float get_height() const;

		bool is_key_pressed(int key_code) const;

		void update();

		static void block_until_event_happens();
	protected:
		void handle_key_event(int key, int scancode, int action, int mods);

		GLFWwindow* wnd;
		std::vector<std::function<WindowResizeSignature>> window_resize_callbacks;
	private:
		std::pair<int, int> get_size() const;
		void ensure() const;

		KeyPressedState pressed;
	};

	/**
	 * @}
	 */
}

#include "core/window_functionality.inl"
#endif // TOPAZ_CORE_WINDOW_FUNCTIONALITY_HPP
