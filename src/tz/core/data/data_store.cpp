#include "tz/core/data/data_store.hpp"
#include "tz/core/profile.hpp"
#include <mutex>

namespace tz
{
	void data_store::clear()
	{
		std::unique_lock<mutex> ulock(this->mtx);
		this->store.clear();
	}

	void data_store::add(detail::ds_add add)
	{
		TZ_PROFZONE("data_store - single add", 0xFF3377AA);
		std::unique_lock<mutex> ulock(this->mtx);
		tz::assert(!this->contains_nolock(add.key), "add called on %s which already exists in the datastore.", add.key.data());
		this->store[add.key] = add.val;
	}

	void data_store::set(detail::ds_edit edit)
	{
		TZ_PROFZONE("data_store - single set", 0xFF3377AA);
		std::unique_lock<mutex> ulock(this->mtx);
		this->store[edit.key] = edit.val;
	}

	void data_store::edit(detail::ds_edit edit)
	{
		TZ_PROFZONE("data_store - single edit", 0xFF3377AA);
		std::unique_lock<mutex> ulock(this->mtx);
		tz::assert(this->contains_nolock(edit.key), "edit called on %s which does not exist in the datastore.", edit.key.data());
		this->store[edit.key] = edit.val;
	}

	void data_store::remove(detail::ds_remove remove)
	{
		TZ_PROFZONE("data_store - remove", 0xFF3377AA);
		std::unique_lock<mutex> ulock(this->mtx);
		tz::assert(this->contains_nolock(remove.key), "remove called on %s, which does not exist in the datastore", remove.key.data());
		this->store.erase(remove.key);
	}

	void data_store::remove_all_of(std::string_view prefix)
	{
		TZ_PROFZONE("data_store - remove all of", 0xFF3377AA);
		std::unique_lock<mutex> ulock(this->mtx);
		for(auto iter = this->store.begin(); iter != this->store.end();)
		{
			if(iter->first.starts_with(prefix))
			{
				TZ_PROFZONE("remove all of - erase", 0xFF3377AA);
				iter = this->store.erase(iter);
			}
			else
			{
				iter++;
			}
		}
	}

	// helper type for the visitor #4
	template<class... Ts>
	struct overloaded : Ts... { using Ts::operator()...; };
	// explicit deduction guide (not needed as of C++20)
	template<class... Ts>
	overloaded(Ts...) -> overloaded<Ts...>;

	void data_store::write_some(deferred_operations operations)
	{
		TZ_PROFZONE("data_store - bulk write", 0xFF3377AA);
		// pre-lock uniquely and we're free to make all the edits we like.
		std::unique_lock<mutex> ulock(this->mtx);
		for(auto op : operations)
		{
			TZ_PROFZONE("data_store - generic operation", 0xFF3377AA);
			std::visit(overloaded
			{
				[]([[maybe_unused]] auto arg){tz::error("Unknown variant value within bulk_write. Memory corruption? Please submit a bug report.");},
				[this](detail::ds_add add)
				{
					this->store[add.key] = add.val;
				},
				[this](detail::ds_edit edit)
				{
					//tz::assert(this->contains_nolock(edit.key), "edit called on %s which does not exist in the datastore.", edit.key.data());
					this->store[edit.key] = edit.val;
				},
				[this](detail::ds_remove remove)
				{
					tz::assert(this->contains_nolock(remove.key), "remove called on %s, which does not exist in the datastore", remove.key.data());
					this->store.erase(remove.key);
				}
			}, op);
		}
	}

	data_store::bulk_read_result data_store::read_some(data_store::string_list keyset, bool dont_error_if_missing) const
	{
		TZ_PROFZONE("data_store - bulk read", 0xFF3377AA);
		std::shared_lock<mutex> slock(this->mtx);
		data_store::bulk_read_result ret;
		ret.resize(keyset.size(), nullptr);
		for(std::size_t i = 0; i < keyset.size(); i++)
		{
			TZ_PROFZONE("data_store - read one", 0xFF3377AA);
			auto iter = this->store.find(keyset[i]);
			if(iter != this->store.end())
			{
				ret[i] = iter->second;	
			}
			else
			{
				tz::assert(!dont_error_if_missing, "`read` called (via bulk read) on %s, which does not exist in the datastore.", keyset[i].data());
			}
		}
		return ret;
	}

	bool data_store::contains(std::string_view key) const
	{
		std::shared_lock<mutex> slock(this->mtx);
		return this->contains_nolock(key);
	}

	std::size_t data_store::size() const
	{
		std::shared_lock<mutex> slock(this->mtx);
		return this->store.size();
	}

	data_store_value data_store::read_raw(std::string_view key, bool dont_error_if_missing) const
	{
		TZ_PROFZONE("data_store - read raw", 0xFF3377AA);
		std::shared_lock<mutex> slock(this->mtx);
		if(this->contains_nolock(key))
		{
			return this->store.at(std::string{key});
		}
		if(!dont_error_if_missing)
		{
			tz::error("`read_raw` called on %s, which does not exist in the datastore.", key.data());
		}
		return nullptr;
	}

	bool data_store::contains_nolock(std::string_view key) const
	{
		return this->store.contains(std::string{key});
	}

	// lua api
	////////////////////

	int tz_lua_data_store::add(tz::lua::state& state)
	{
		std::string key = state.stack_get_string(2);
		tz::lua::lua_generic val = state.stack_get_generic(3);
		std::visit(overloaded
		{
			[]([[maybe_unused]] auto arg){tz::error("Lua value type passed to data_store.add is unknown or invalid. Please submit a bug report.");},
			[this, key](bool b)
			{
				this->ds->add({.key = key, .val = b});
			},
			[this, key](std::int64_t i)
			{
				this->ds->add({.key = key, .val = static_cast<int>(i)});
			},
			[this, key](double d)
			{
				this->ds->add({.key = key, .val = d});
			},
			[this, key](std::string str)
			{
				this->ds->add({.key = key, .val = str});
			},
		}, val);
		return 0;
	}

	int tz_lua_data_store::set(tz::lua::state& state)
	{
		std::string key = state.stack_get_string(2);
		tz::lua::lua_generic val = state.stack_get_generic(3);
		std::visit(overloaded
		{
			[]([[maybe_unused]] auto arg){tz::error("Lua value type passed to data_store.set is unknown or invalid. Please submit a bug report.");},
			[this, key](bool b)
			{
				this->ds->set({.key = key, .val = b});
			},
			[this, key](double d)
			{
				this->ds->set({.key = key, .val = d});
			},
			[this, key](std::string str)
			{
				this->ds->set({.key = key, .val = str});
			},
		}, val);
		return 0;
	}

	int tz_lua_data_store::edit(tz::lua::state& state)
	{
		std::string key = state.stack_get_string(2);
		tz::lua::lua_generic val = state.stack_get_generic(3);
		std::visit(overloaded
		{
			[]([[maybe_unused]] auto arg){tz::error("Lua value type passed to data_store.edit is unknown or invalid. Please submit a bug report.");},
			[this, key](bool b)
			{
				this->ds->edit({.key = key, .val = b});
			},
			[this, key](double d)
			{
				this->ds->edit({.key = key, .val = d});
			},
			[this, key](std::string str)
			{
				this->ds->edit({.key = key, .val = str});
			},
		}, val);
		return 0;
	}

	int tz_lua_data_store::edit_some(tz::lua::state& state)
	{
		std::size_t arg_count = state.stack_size() - 1;
		data_store::string_list keys(arg_count / 2);
		std::vector<lua::lua_generic> values(arg_count / 2);
		for(std::size_t i = 0; (i + 1) < arg_count; i += 2)
		{
			keys[i / 2] = state.stack_get_string(i + 1 + 1);
			values[i / 2] = state.stack_get_generic(i + 1 + 1 + 1);
		}
		data_store::deferred_operations ops;
		for(std::size_t i = 0; i < arg_count / 2; i++)
		{
			data_store_value v;
			std::visit(overloaded
			{
				[]([[maybe_unused]] auto arg){tz::error("Lua value type passed to data_store.edit is unknown or invalid. Please submit a bug report.");},
				[&v, key = keys[i]](bool b)
				{
					v = b;
				},
				[&v, key = keys[i]](double d)
				{
					v = d;
				},
				[&v, key = keys[i]](float f)
				{
					v = f;
				},
				[&v, key = keys[i]](std::int64_t i)
				{
					v = static_cast<int>(i);
				},
				[&v, key = keys[i]](std::uint64_t u)
				{
					v = static_cast<unsigned int>(u);
				},
				[&v, key = keys[i]](std::string str)
				{
					v = str;
				},
			}, values[i]);
			ops.push_back(detail::ds_edit({.key = keys[i], .val = v}));
		}
		this->ds->write_some(ops);
		return 0;
	}

	int tz_lua_data_store::remove(tz::lua::state& state)
	{
		std::string key = state.stack_get_string(2);
		this->ds->remove({.key = key});
		return 0;
	}

	int tz_lua_data_store::remove_all_of(tz::lua::state& state)
	{
		std::string key = state.stack_get_string(2);
		this->ds->remove_all_of(key);
		return 0;
	}

	int tz_lua_data_store::read(tz::lua::state& state)
	{
		std::string key = state.stack_get_string(2);
		data_store_value val = this->ds->read_raw(key, true);
		std::visit(overloaded
		{
			[&state]([[maybe_unused]] auto arg){state.stack_push_nil();},
			[&state](bool b){state.stack_push_bool(b);},
			[&state](float b){state.stack_push_float(b);},
			[&state](double b){state.stack_push_double(b);},
			[&state](int b){state.stack_push_int(b);},
			[&state](unsigned int b){state.stack_push_uint(b);},
			[&state](std::string b){state.stack_push_string(b);},
		}, val);
		return 1;
	}

	int tz_lua_data_store::read_some(tz::lua::state& state)
	{
		std::size_t arg_count = state.stack_size() - 1;
		data_store::string_list keys(arg_count);
		for(std::size_t i = 0; i < arg_count; i++)
		{
			keys[i] = state.stack_get_string(i + 1 + 1);
		}
		data_store::bulk_read_result result = this->ds->read_some(keys);
		tz::assert(result.size() == arg_count);
		for(data_store_value val : result)
		{
			std::visit(overloaded
			{
				[&state]([[maybe_unused]] auto arg){state.stack_push_nil();},
				[&state](bool b){state.stack_push_bool(b);},
				[&state](float b){state.stack_push_float(b);},
				[&state](double b){state.stack_push_double(b);},
				[&state](int b){state.stack_push_int(b);},
				[&state](unsigned int b){state.stack_push_uint(b);},
				[&state](std::string b){state.stack_push_string(b);},
			}, val);
		}
		return result.size();
	}

	int tz_lua_data_store::contains(tz::lua::state& state)
	{
		std::string key = state.stack_get_string(2);
		state.stack_push_bool(this->ds->contains(key));
		return 1;
	}

	int tz_lua_data_store::size(tz::lua::state& state)
	{
		state.stack_push_uint(this->ds->size());
		return 1;
	}

	int tz_lua_data_store::clear(tz::lua::state& state)
	{
		(void)state;
		this->ds->clear();
		return 1;
	}
}