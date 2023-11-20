#ifndef TZ_LUA_STATE_HPP
#define TZ_LUA_STATE_HPP
#include "tz/core/data/handle.hpp"
#include "tz/core/memory/memblk.hpp"
#include <string>
#include <cstdint>
#include <type_traits>
#include <functional>
#include <optional>
#include <thread>
#include <span>

namespace tz::lua
{
	struct nil{};
	namespace impl
	{
		using fn_t = int(*)(void*);
		struct lua_register
		{
			const char* namestr;
			fn_t fnptr; // signature: int(void*)
		};
		using lua_registers = std::span<const lua_register>;
	}

	/**
	* @ingroup tz_lua_cpp
	* Represents a lua state. To retrieve the main lua state, see @ref tz::lua::get_state()
	*/
	class state
	{
	public:
		state() = default;
		state(void* lstate);
		/**
		* Query as to whether the state is valid.
		* @return True if the state is valid, false otherwise.
		*/
		bool valid() const;
		/**
		* Attempt to execute a lua source file. The code is instantly executed, and returns on completion.
		* @param path The relative path locating the lua source file (with extension).
		* @param assert_on_failure Whether Topaz should assert on the code running without any errors.
		* @return Whether the executed code experienced any errors.
		*/
		bool execute_file(const char* path, bool assert_on_failure = true) const;
		/**
		* Attempt to execute a lua source string. The code is instantly executed, and returns on completion.
		* @param lua_src String containing lua source code, appropriate for the state.
		* @param assert_on_failure Whether Topaz should assert on the code running without any errors.
		* @return Whether the executed code experienced any errors.
		*/
		bool execute(const char* lua_src, bool assert_on_failure = true) const;

		// assigning variables
		bool assign_nil(const char* varname) const;
		bool assign_emptytable(const char* varname) const;
		bool assign_bool(const char* varname, bool b) const;
		bool assign_float(const char* varname, float f) const;
		bool assign_double(const char* varname, double d) const;
		bool assign_int(const char* varname, std::int64_t i) const;
		bool assign_uint(const char* varname, std::uint64_t u) const;
		bool assign_func(const char* varname, auto anon_ptr) const
		{
			using T = std::decay_t<decltype(anon_ptr)>;
			if constexpr(std::is_pointer_v<T>)
			{
				return this->assign_func(varname, reinterpret_cast<void*>(anon_ptr));
			}
			return false;
		}
		bool assign_func(const char* varname, void* func_ptr) const;
		bool assign_string(const char* varname, std::string str) const;
		void assign_stack(const char* varname);
		// retrieving variable values
		std::optional<bool> get_bool(const char* varname) const;
		std::optional<float> get_float(const char* varname) const;
		std::optional<double> get_double(const char* varname) const;
		std::optional<std::int64_t> get_int(const char* varname) const;
		std::optional<std::uint64_t> get_uint(const char* varname) const;
		std::optional<std::string> get_string(const char* varname) const;
		// stack operations
		std::size_t stack_size() const;
		void stack_pop(std::size_t count = 1);
		// read a value off of the stack
		bool stack_get_bool(std::size_t idx, bool type_check = true) const;
		double stack_get_double(std::size_t idx, bool type_check = true) const;
		float stack_get_float(std::size_t idx, bool type_check = true) const;
		std::int64_t stack_get_int(std::size_t idx, bool type_check = true) const;
		std::uint64_t stack_get_uint(std::size_t idx, bool type_check = true) const;
		std::string stack_get_string(std::size_t idx, bool type_check = true) const;
		void* stack_get_ptr(std::size_t idx, bool type_check = true) const;
		template<typename T>
		T& stack_get_userdata(std::size_t idx, bool type_check = true) const
		{
			return *reinterpret_cast<T*>(this->stack_get_ptr(idx, type_check));
		}
		// push a new value onto the stack
		void stack_push_nil() const;
		void stack_push_bool(bool b) const;
		void stack_push_double(double d) const;
		void stack_push_float(float f) const;
		void stack_push_int(std::int64_t i) const;
		void stack_push_uint(std::uint64_t u) const;
		void stack_push_string(std::string_view sv) const;
		// light user data
		void stack_push_ptr(void* ptr) const;
		template<typename T>
		void stack_push_ref(T& t) const
		{
			stack_push_ptr(&t);
		}
		template<typename T>
		void stack_push_userdata(const T& t)
		{
			constexpr std::size_t sz = sizeof(T);
			auto blk = lua_userdata_stack_push(sz);
			new (blk.ptr) T{t};
		}

		std::string collect_stack() const;
		std::string print_traceback() const;
		const std::string& get_last_error() const;
		std::thread::id get_owner_thread_id() const;
		void attach_to_top_userdata(const char* classname, impl::lua_registers registers);
		void attach_to_top_table(impl::lua_registers registers);
		void new_type(const char* type_name, impl::lua_registers registers);
		void open_lib(const char* name, impl::lua_registers registers);
		void* operator()() const;
	private:
		tz::memblk lua_userdata_stack_push(std::size_t byte_count) const;
		bool impl_check_stack(std::size_t sz) const;
		mutable std::string last_error = "";
		void* lstate = nullptr;
		std::thread::id owner = std::this_thread::get_id();
	};

	/**
	* @ingroup tz_lua_cpp
	* Retrieve the main lua @ref tz::lua::state.
	*/
	state& get_state();

	using state_applicator = std::function<void(state&)>;
	void for_all_states(state_applicator fn);

}

#endif // TZ_LUA_STATE_HPP
