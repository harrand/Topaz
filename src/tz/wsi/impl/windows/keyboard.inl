#include "tz/wsi/impl/windows/detail/winapi.hpp"

namespace tz::wsi::impl
{
	constexpr tz::wsi::key win_to_tge_key(int virtual_key_code)
	{
		tz::wsi::key k = tz::wsi::key::unknown;
		switch(virtual_key_code)
		{
			case VK_ESCAPE:
				k = tz::wsi::key::esc;
			break;
			case VK_F1:
				k = tz::wsi::key::f1;
			break;
			case VK_F2:
				k = tz::wsi::key::f2;
			break;
			case VK_F3:
				k = tz::wsi::key::f3;
			break;
			case VK_F4:
				k = tz::wsi::key::f4;
			break;
			case VK_F5:
				k = tz::wsi::key::f5;
			break;
			case VK_F6:
				k = tz::wsi::key::f6;
			break;
			case VK_F7:
				k = tz::wsi::key::f7;
			break;
			case VK_F8:
				k = tz::wsi::key::f8;
			break;
			case VK_F9:
				k = tz::wsi::key::f9;
			break;
			case VK_F10:
				k = tz::wsi::key::f10;
			break;
			case VK_F11:
				k = tz::wsi::key::f11;
			break;
			case VK_F12:
				k = tz::wsi::key::f12;
			break;
			case VK_DELETE:
				k = tz::wsi::key::del;
			break;
			case VK_PRIOR:
				k = tz::wsi::key::page_up;
			break;
			case VK_NEXT:
				k = tz::wsi::key::page_down;
			break;
			case VK_SNAPSHOT:
				k = tz::wsi::key::print_screen;
			break;
			case VK_SCROLL:
				k = tz::wsi::key::scroll_lock;
			break;
			case VK_PAUSE:
				k = tz::wsi::key::pause;
			break;
			case VK_INSERT:
				k = tz::wsi::key::insert;
			break;
			case 0x31:
				k = tz::wsi::key::one;
			break;
			case 0x32:
				k = tz::wsi::key::two;
			break;
			case 0x33:
				k = tz::wsi::key::three;
			break;
			case 0x34:
				k = tz::wsi::key::four;
			break;
			case 0x35:
				k = tz::wsi::key::five;
			break;
			case 0x36:
				k = tz::wsi::key::six;
			break;
			case 0x37:
				k = tz::wsi::key::seven;
			break;
			case 0x38:
				k = tz::wsi::key::eight;
			break;
			case 0x39:
				k = tz::wsi::key::nine;
			break;
			case 0x30:
				k = tz::wsi::key::zero;
			break;
			case VK_OEM_MINUS:
				k = tz::wsi::key::minus;
			break;
			case VK_OEM_PLUS:
				k = tz::wsi::key::equals; // not a typo...
			break;
			case VK_BACK:
				k = tz::wsi::key::backspace;
			break;
			case VK_TAB:
				k = tz::wsi::key::tab;
			break;
			case 0x51:
				k = tz::wsi::key::q;
			break;
			case 0x57:
				k = tz::wsi::key::w;
			break;
			case 0x45:
				k = tz::wsi::key::e;
			break;
			case 0x52:
				k = tz::wsi::key::r;
			break;
			case 0x54:
				k = tz::wsi::key::t;
			break;
			case 0x59:
				k = tz::wsi::key::y;
			break;
			case 0x55:
				k = tz::wsi::key::u;
			break;
			case 0x49:
				k = tz::wsi::key::i;
			break;
			case 0x4F:
				k = tz::wsi::key::o;
			break;
			case 0x50:
				k = tz::wsi::key::p;
			break;
			case VK_OEM_4:
				k = tz::wsi::key::left_bracket;
			break;
			case VK_OEM_6:
				k = tz::wsi::key::right_bracket;
			break;
			case VK_RETURN:
				k = tz::wsi::key::enter;
			break;
			case VK_CAPITAL:
				k = tz::wsi::key::caps_lock;
			break;
			case 0x41:
				k = tz::wsi::key::a;
			break;
			case 0x53:
				k = tz::wsi::key::s;
			break;
			case 0x44:
				k = tz::wsi::key::d;
			break;
			case 0x46:
				k = tz::wsi::key::f;
			break;
			case 0x47:
				k = tz::wsi::key::g;
			break;
			case 0x48:
				k = tz::wsi::key::h;
			break;
			case 0x4A:
				k = tz::wsi::key::j;
			break;
			case 0x4B:
				k = tz::wsi::key::k;
			break;
			case 0x4C:
				k = tz::wsi::key::l;
			break;
			case VK_OEM_1:
				k = tz::wsi::key::semi_colon;
			break;
			case VK_OEM_3:
				k = tz::wsi::key::apostrophe;
			break;
			case VK_OEM_7:
				k = tz::wsi::key::hash;
			break;
			case VK_SHIFT:
				k = tz::wsi::key::left_shift;
			break;
			case VK_OEM_5:
				k = tz::wsi::key::backslash;
			break;
			case 0x5A:
				k = tz::wsi::key::z;
			break;
			case 0x58:
				k = tz::wsi::key::x;
			break;
			case 0x43:
				k = tz::wsi::key::c;
			break;
			case 0x56:
				k = tz::wsi::key::v;
			break;
			case 0x42:
				k = tz::wsi::key::b;
			break;
			case 0x4E:
				k = tz::wsi::key::n;
			break;
			case 0x4D:
				k = tz::wsi::key::m;
			break;
			case VK_OEM_COMMA:
				k = tz::wsi::key::comma;
			break;
			case VK_OEM_PERIOD:
				k = tz::wsi::key::period;
			break;
			case VK_OEM_2:
				k = tz::wsi::key::forward_slash;
			break;
			case VK_RSHIFT:
				k = tz::wsi::key::right_shift;
			break;
			case VK_CONTROL:
				k = tz::wsi::key::left_ctrl;
			break;
			case VK_LWIN:
				k = tz::wsi::key::win_key;
			break;
			case VK_MENU:
				k = tz::wsi::key::alt;
			break;
			case VK_SPACE:
				k = tz::wsi::key::space;
			break;
			case VK_RMENU:
				k = tz::wsi::key::alt_gr;
			break;
			case VK_RCONTROL:
				k = tz::wsi::key::right_ctrl;
			break;
		}
		return k;
	}

	constexpr int tge_to_win_key(tz::wsi::key key)
	{
		// We have a conversion the other way around, so let's cheat. We're constexpr so we're allowed to be mega slow.
		for(int i = 0x01; i < 0xFF; i++)
		{
			if(win_to_tge_key(i) == key)
			{
				return i;
			}
		}
		return 0x07; // there are a couple of virtual keycodes which are undefined. 0x07 is one of them.
	}
}
