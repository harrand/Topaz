#ifndef TOPAZ_CORE_DATA_DATA_STORE_HPP
#define TOPAZ_CORE_DATA_DATA_STORE_HPP
#include "tz/core/memory/memblk.hpp"
#include "tz/core/debug.hpp"
#include <shared_mutex>
#include <variant>
#include <vector>
#include <unordered_map>

#include "tz/lua/api.hpp"

namespace tz
{
	using data_store_value = std::variant<nullptr_t, bool, float, double, int, unsigned int, std::string>;
	namespace detail
	{
		struct ds_add
		{
			std::string key;
			data_store_value val = nullptr;
		};
		struct ds_edit
		{
			std::string key;
			data_store_value val;
		};
		struct ds_remove
		{
			std::string key;
		};
	}

	/*
		fully thread-safe data store. key-value storage, where key is a string, and value is one of:
		- nullptr (i.e null)
		- bool
		- float
		- double
		- int
		- unsigned int
		- std::string

		why would you use this
		======================

		only reason: you have absolutely no choice but to take the hit of shared data between threads. use this to bring at least a semblance of sanity to your life

		the most obvious example of this would be lua. each job system worker thread has their own lua state.
		most likely you'll want to share some common data between these threads. that's pretty much a perfect use-case for this datastore.

		tradeoffs
		=========
		
		- all writes (add, edit, remove, clear) exclusively lock a mutex. all other threads that want to read/write wait for this to complete.
		- all reads sharingly lock a mutex. threads can safely read concurrently, each sharingly locking the mutex. if a thread wants to write, it will wait.

		tips
		====

		- avoid repeatedly calling add/edit/remove, instead bulk your mutating operations together in `write_some`. less write churn means less lock contention means better perf.
		- avoid repeatedly calling read/read_raw, instead builk your reading operations together in `read_some`.

		rejected features
		=================

		- non-deferred listeners i.e callbacks. don't do it! please no! oh god!
	*/
	class data_store
	{
	public:
		using deferred_operation = std::variant<detail::ds_add, detail::ds_edit, detail::ds_remove>;
		using deferred_operations = std::vector<deferred_operation>;
		using bulk_read_result = std::vector<data_store_value>;
		using string_list = std::vector<std::string>;
		data_store() = default;
		void clear();
		void add(detail::ds_add add);
		void edit(detail::ds_edit edit);
		void remove(detail::ds_remove remove);
		void remove_all_of(std::string_view prefix);
		void write_some(deferred_operations operations);
		bulk_read_result read_some(string_list keyset, bool dont_error_if_missing = false) const;

		bool contains(std::string_view key) const;
		std::size_t size() const;
		data_store_value read_raw(std::string_view key, bool dont_error_if_missing = false) const;

		// read as T. error if value is not a T.
		template<typename T>
		T read(std::string_view key) const
		{
			data_store_value val = this->read_raw(key);
			tz::assert(std::holds_alternative<T>(val), "`read` called on %s, but there was a type mismatch");
			return std::get<T>(val);
		}

		// read and attempt to convert to T, if it can.
		// e.g if 6.9f is stored, read_as<int>(key) returns 6
		template<typename T>
		T read_as(std::string_view key) const
		{
			data_store_value val = this->read_raw(key);
			T ret;
			std::visit([&ret](auto&& arg)
			{
				ret = static_cast<T>(arg);
			}, val);
		}
	private:
		bool contains_nolock(std::string_view key) const;

		using mutex = std::shared_mutex;
		mutable mutex mtx;
		std::unordered_map<std::string, data_store_value> store{1024u};
	};

	// lua api
	struct tz_lua_data_store
	{
		data_store* ds;
		int add(tz::lua::state& state);
		int edit(tz::lua::state& state);
		int edit_some(tz::lua::state& state);
		int remove(tz::lua::state& state);
		int remove_all_of(tz::lua::state& state);
		int read(tz::lua::state& state);
		int read_some(tz::lua::state& state);
		int contains(tz::lua::state& state);
		int size(tz::lua::state& state);
		int clear(tz::lua::state& state);
	};

	LUA_CLASS_BEGIN(tz_lua_data_store)
		LUA_CLASS_METHODS_BEGIN
			LUA_METHOD(tz_lua_data_store, add)
			LUA_METHOD(tz_lua_data_store, edit)
			LUA_METHOD(tz_lua_data_store, edit_some)
			LUA_METHOD(tz_lua_data_store, remove)
			LUA_METHOD(tz_lua_data_store, remove_all_of)
			LUA_METHOD(tz_lua_data_store, read)
			LUA_METHOD(tz_lua_data_store, read_some)
			LUA_METHOD(tz_lua_data_store, contains)
			LUA_METHOD(tz_lua_data_store, size)
			LUA_METHOD(tz_lua_data_store, clear)
		LUA_CLASS_METHODS_END
	LUA_CLASS_END

}

#endif // TOPAZ_CORE_DATA_DATA_STORE_HPP