#ifndef TOPAZ_CORE_DATA_DATA_STORE_HPP
#define TOPAZ_CORE_DATA_DATA_STORE_HPP
#include "tz/core/memory/memblk.hpp"
#include "tz/core/debug.hpp"
#include <shared_mutex>
#include <variant>
#include <vector>
#include <unordered_map>

namespace tz
{
	using data_store_value = std::variant<nullptr_t, bool, float, double, int, unsigned int, std::string>;
	namespace detail
	{
		struct ds_add
		{
			std::string_view key;
			data_store_value val = nullptr;
		};
		struct ds_edit
		{
			std::string_view key;
			data_store_value val;
		};
		struct ds_remove
		{
			std::string_view key;
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
	*/
	class data_store
	{
	public:
		using deferred_operation = std::variant<detail::ds_add, detail::ds_edit, detail::ds_remove>;
		using deferred_operations = std::vector<deferred_operation>;
		using bulk_read_result = std::vector<data_store_value>;
		using string_list = std::vector<std::string_view>;
		data_store() = default;
		void clear();
		void add(detail::ds_add add);
		void edit(detail::ds_edit edit);
		void remove(detail::ds_remove remove);
		void write_some(deferred_operations operations);
		bulk_read_result read_some(string_list keyset, bool dont_error_if_missing = false) const;

		bool contains(std::string_view key) const;
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
		using mutex = std::shared_mutex;
		mutable mutex mtx;
		std::unordered_map<std::string_view, data_store_value> store{1024u};
	};
}

#endif // TOPAZ_CORE_DATA_DATA_STORE_HPP