#ifndef TOPAZ_GL_window_HPP
#define TOPAZ_GL_window_HPP
#include "imgui.h"
#include <memory>


// Forward Declares
struct GLFWwindow;
namespace tz::gl
{
	class Object;
}

namespace tz::dui
{
	class DebugWindow
	{
	public:
		DebugWindow(const char* name);
		const char* get_name() const;
		virtual void render() = 0;

		bool visible;
	private:
		const char* name;
	};

	[[maybe_unused]] static GLFWwindow* window;
	void set_window_impl(GLFWwindow* wnd);
	void track_object(tz::gl::Object* obj);
	void initialise();
	void terminate();
	template<class WindowType, typename... Args>
	WindowType& emplace_window(Args&&... args);
	DebugWindow& add_window(std::unique_ptr<DebugWindow> wnd);
	void update();
}

#include "dui/window.inl"
#endif // TOPAZ_GL_window_HPP