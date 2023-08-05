#ifndef TZ_LUA_STATE_HPP
#define TZ_LUA_STATE_HPP
#include "tz/core/data/handle.hpp"

namespace tz::lua
{
	class state
	{
	public:
		state() = default;
		state(void* lstate);
		bool valid() const;
		bool execute_file(const char* path, bool assert_on_failure = true) const;
		bool execute(const char* lua_src, bool assert_on_failure = true) const;
	private:
		void* lstate = nullptr;
	};

	state& get_state();
}

#endif // TZ_LUA_STATE_HPP
