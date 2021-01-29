//
// Created by Harrand on 13/12/2019.
//

#include "input/input_listener.hpp"
#include "input/system_input.hpp"
#include "core/core.hpp"
#include "window_demo.hpp"
#include "core/debug/assert.hpp"
#include "core/debug/print.hpp"
#include "core/random.hpp"
#include "GLFW/glfw3.h"

WindowDemo::WindowDemo(): window(nullptr), second_window_id(std::nullopt), second_context(nullptr)
{
	tz::core::initialise("WindowDemo (Press 'Q' to spawn a second window and 'W' to destroy it)");
	tz::ext::glfw::get().set_error_callback([](int err, const char* err_msg){tz::debug_printf("oh noes!");});
	tz::debug_printf("Project Source Dir: \"%s\"\n", tz::core::project_directory);
	this->window = &tz::core::get().window();
	this->window->register_this();
	this->window->emplace_custom_key_listener(
			[this](tz::input::KeyPressEvent kpe)
			{
				if(kpe.action == GLFW_PRESS || kpe.action == GLFW_REPEAT)
				{
					if (kpe.key == GLFW_KEY_ESCAPE)
					{
						this->window->request_close();
					}
					
					if(kpe.key == GLFW_KEY_Q)
					{
						this->create_second_window();
					}
					
					if(kpe.key == GLFW_KEY_W)
					{
						this->destroy_second_window();
					}

					if(kpe.key == GLFW_KEY_R)
					{
						tz::debug_printf("\n%g\n", tz::range(-10.0f, 10.0f));
					}
				}
			}
	);
	this->window->emplace_custom_type_listener(
			[](tz::input::CharPressEvent cpe)
			{
				tz::debug_printf("%c", cpe.get_char());
			}
	);
	this->window->emplace_custom_mouse_listener(
			[](tz::input::MouseUpdateEvent mue)
			{
				//tz::debug_printf("mouse pos: [%g, %g]\n", mue.xpos, mue.ypos);
			}
			,
			[](tz::input::MouseClickEvent mce)
			{
				if(mce.action == GLFW_PRESS)
				{
					tz::debug_printf("mouse click: %d, %d, %d\n", mce.button, mce.action, mce.mods);
				}
			}
	);
}

WindowDemo::~WindowDemo()
{
	tz::core::terminate();
}

bool WindowDemo::playing() const
{
	topaz_assert(this->window != nullptr, "WindowDemo::playing(): Underlying window handle is not set! Did you forget to tz::initialise()?");
	return !this->window->is_close_requested();
}

void WindowDemo::update()
{
	topaz_assert(this->window != nullptr, "");
	this->window->update();
	if(this->second_window_id.has_value())
	{
		tz::core::IWindow* second_window = tz::core::get().get_extra_window(this->second_window_id.value());
		if(second_window != nullptr)
		{
			second_window->update();
			if(second_window->is_close_requested())
			{
				this->destroy_second_window();
			}
		}
	}
}

void WindowDemo::create_second_window()
{
	topaz_assert(!this->second_window_id.has_value(), "WindowDemo::create_second_window(): Second window already exists!");
	tz::ext::glfw::WindowCreationArgs second_args{};
	this->second_context = &tz::ext::glfw::make_secondary_context(second_args);
	this->second_window_id = {tz::core::get().emplace_extra_window<tz::core::GLFWWindow>(*this->second_context)};
	tz::core::IWindow* second_wnd = tz::core::get().get_extra_window(this->second_window_id.value());
	second_wnd->emplace_custom_type_listener([this](tz::input::CharPressEvent cpe)
											 {
												 tz::core::IWindow* wnd = tz::core::get().get_extra_window(this->second_window_id.value());
												 std::string s = wnd->get_title();
												 s += static_cast<char>(cpe.codepoint);
												 wnd->set_title(s.c_str());
											 });
	second_wnd->emplace_custom_key_listener([this](tz::input::KeyPressEvent kpe)
											{
												if(kpe.key == GLFW_KEY_ESCAPE)
												{
													this->destroy_second_window();
												}

												if(kpe.key == GLFW_KEY_V && (kpe.mods & GLFW_MOD_CONTROL))
												{
													
												}

												if(kpe.key == GLFW_KEY_BACKSPACE && (kpe.action == GLFW_PRESS || kpe.action == GLFW_REPEAT))
												{
													tz::core::IWindow* wnd = tz::core::get().get_extra_window(this->second_window_id.value());
													std::string s = wnd->get_title();
													if(s.empty())
														return;
													if(kpe.mods & GLFW_MOD_CONTROL)
													{
														std::size_t idx = s.find_last_of(' ');
														if(idx == std::string::npos)
															s.clear();
														else
															s.erase(idx, s.size() - idx);
													}
													else
													{
														s.pop_back();
													}
													wnd->set_title(s.c_str());
												}
											});
	second_wnd->set_title("Type anything to change my title! Or press Escape to close me.");
}

void WindowDemo::destroy_second_window()
{
	topaz_assert(this->second_window_id.has_value(), "WindowDemo::destroy_second_window(): Second window didn't exist!");
	tz::core::get().destroy_extra_window(this->second_window_id.value());
	this->second_window_id = {std::nullopt};
	tz::ext::glfw::destroy_secondary_context(*this->second_context);
	this->second_context = nullptr;
}

int main(int argc, char** argv)
{
	WindowDemo demo;
	while(demo.playing())
	{
		demo.update();
		tz::core::update();
	}
	return 0;
}