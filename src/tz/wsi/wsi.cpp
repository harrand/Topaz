#include "tz/wsi/wsi.hpp"
#include "tz/dbgui/dbgui.hpp"
#include "tz/lua/api.hpp"
#include "tz/tz.hpp"

#include "tz/core/imported_text.hpp"
#include ImportedTextHeader(tz_wsi, lua)

#include "tz/wsi/window.hpp"
#include "tz/wsi/keyboard.hpp"
#include "tz/wsi/mouse.hpp"

#include "tz/wsi/impl/windows/wsi_windows.hpp"
#include "tz/wsi/impl/linux/wsi_linux.hpp"
#include <algorithm>
#include <cctype>

namespace tz::wsi
{
	void initialise()
	{
		#ifdef _WIN32
			tz::wsi::impl::initialise_windows();
		#elif defined(__linux__)
			tz::wsi::impl::initialise_linux();	
		#endif
	}
	
	void terminate()
	{
		#ifdef _WIN32
			tz::wsi::impl::terminate_windows();
		#elif defined(__linux__)
			tz::wsi::impl::terminate_linux();	
		#endif
	}

	void update()
	{
		#ifdef _WIN32
			tz::wsi::impl::update_windows();
		#elif defined(__linux__)
			tz::wsi::impl::update_linux();	
		#endif
	}


	// LUA API
	struct impl_tz_wsi_window
	{
		wsi::window* wnd = nullptr;
		int get_title(tz::lua::state& state)
		{
			tz::assert(this->wnd != nullptr);
			state.stack_push_string(wnd->get_title());
			return 1;
		}

		int set_title(tz::lua::state& state)
		{
			tz::assert(this->wnd != nullptr);
			auto [_, new_title] = tz::lua::parse_args<tz::lua::nil, std::string>(state);
			this->wnd->set_title(new_title);
			return 0;
		}

		int get_dimensions(tz::lua::state& state)
		{
			tz::assert(this->wnd != nullptr);
			tz::vec2ui dims = this->wnd->get_dimensions();
			state.stack_push_uint(dims[0]);
			state.stack_push_uint(dims[1]);
			return 2;
		}

		int set_dimensions(tz::lua::state& state)
		{
			tz::assert(this->wnd != nullptr);
			auto [_, w, h] = tz::lua::parse_args<tz::lua::nil, unsigned int, unsigned int>(state);
			this->wnd->set_dimensions(tz::vec2ui{w, h});
			return 0;
		}

		int is_key_down(tz::lua::state& state)
		{
			tz::assert(this->wnd != nullptr);
			auto [_, keystr] = tz::lua::parse_args<tz::lua::nil, std::string>(state);
			std::transform(keystr.begin(), keystr.end(), keystr.begin(), [](char c){return std::tolower(c);});
			bool ret = false;
			for(std::size_t i = 0; i < (std::size_t)tz::wsi::key::_count; i++)
			{
				key k = static_cast<key>(i);
				std::string kname = tz::wsi::get_key_name(k);
				std::transform(kname.begin(), kname.end(), kname.begin(), [](char c){return std::tolower(c);});
				kname.erase(std::remove(kname.begin(), kname.end(), '\0'), kname.end());
				if(kname == keystr)
				{
					if(tz::wsi::is_key_down(this->wnd->get_keyboard_state(), k))
					{
						ret = true;
						break;
					}
				}
			}
			ret &= !tz::dbgui::claims_keyboard();
			state.stack_push_bool(ret);
			return 1;
		}

		int is_mouse_down(tz::lua::state& state)
		{
			tz::assert(this->wnd != nullptr);
			auto [_, button_str] = tz::lua::parse_args<tz::lua::nil, std::string>(state);
			std::transform(button_str.begin(), button_str.end(), button_str.begin(), [](char c){return std::tolower(c);});
			bool down = false;
			if(button_str == "left")
			{
				down = tz::wsi::is_mouse_button_down(this->wnd->get_mouse_state(), tz::wsi::mouse_button::left);
			}
			else if(button_str == "right")
			{
				down = tz::wsi::is_mouse_button_down(this->wnd->get_mouse_state(), tz::wsi::mouse_button::right);
			}
			else if(button_str == "middle")
			{
				down = tz::wsi::is_mouse_button_down(this->wnd->get_mouse_state(), tz::wsi::mouse_button::middle);
			}
			down &= !tz::dbgui::claims_mouse();
			state.stack_push_bool(down);
			return 1;
		}
	};

	LUA_CLASS_BEGIN(impl_tz_wsi_window)
		LUA_CLASS_METHODS_BEGIN
			LUA_METHOD(impl_tz_wsi_window, get_title)
			LUA_METHOD(impl_tz_wsi_window, set_title)
			LUA_METHOD(impl_tz_wsi_window, get_dimensions)
			LUA_METHOD(impl_tz_wsi_window, set_dimensions)
			LUA_METHOD(impl_tz_wsi_window, is_key_down)
			LUA_METHOD(impl_tz_wsi_window, is_mouse_down)
		LUA_CLASS_METHODS_END
	LUA_CLASS_END

	LUA_NAMESPACE_BEGIN(impl_tz_wsi)
		LUA_NAMESPACE_FUNC_BEGIN(window)
			LUA_CLASS_PUSH(state, impl_tz_wsi_window, {.wnd = &tz::window()});
			return 1;
		LUA_NAMESPACE_FUNC_END
	LUA_NAMESPACE_END

	void lua_initialise(tz::lua::state& state)
	{
		state.new_type("impl_tz_wsi_window", LUA_CLASS_NAME(impl_tz_wsi_window)::registers);
		state.open_lib("impl_tz_wsi", LUA_NAMESPACE_NAME(impl_tz_wsi)::registers);
		state.execute("tz.wsi = impl_tz_wsi");
		state.execute("tz.window = function() return tz.wsi.window() end");

		std::string tz_wsi_lua_api{ImportedTextData(tz_wsi, lua)};
		state.execute(tz_wsi_lua_api.data());
	}
}
