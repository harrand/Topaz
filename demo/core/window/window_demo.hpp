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
	void update() const;
private:
	tz::core::IWindow* window;
};


#endif //TOPAZ_WINDOW_DEMO_HPP
