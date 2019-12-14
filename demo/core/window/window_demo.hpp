//
// Created by Harrand on 13/12/2019.
//

#ifndef TOPAZ_WINDOW_DEMO_HPP
#define TOPAZ_WINDOW_DEMO_HPP
#include <cstdio>

// Forward declare
namespace tz::core
{
	class IWindow;
}

class WindowDemo
{
public:
	WindowDemo();
	~WindowDemo();
	bool playing() const;
	void update();
	void create_second_window();
	void destroy_second_window();
private:
	tz::core::IWindow* window;
	std::optional<std::size_t> second_window_id;
	tz::ext::glfw::GLFWContext* second_context;
};


#endif //TOPAZ_WINDOW_DEMO_HPP
