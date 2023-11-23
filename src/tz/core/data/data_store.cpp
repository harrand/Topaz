#include "tz/core/data/data_store.hpp"
#include "tz/core/profile.hpp"

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
		this->store[add.key] = add.val;
	}

	void data_store::edit(detail::ds_edit edit)
	{
		TZ_PROFZONE("data_store - single edit", 0xFF3377AA);
		std::unique_lock<mutex> ulock(this->mtx);
		tz::assert(!this->contains(edit.key), "edit called on %s which does not exist in the datastore.", edit.key.data());
		this->store[edit.key] = edit.val;
	}

	void data_store::remove(detail::ds_remove remove)
	{
		TZ_PROFZONE("data_store - remove", 0xFF3377AA);
		std::unique_lock<mutex> ulock(this->mtx);
		tz::assert(this->contains(remove.key), "remove called on %s, which does not exist in the datastore", remove.key.data());
		this->store.erase(remove.key);
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
				[](auto arg){tz::error("Unknown variant value within bulk_write. Memory corruption? Please submit a bug report.");},
				[this](detail::ds_add add)
				{
					this->store[add.key] = add.val;
				},
				[this](detail::ds_edit edit)
				{
					tz::assert(!this->contains(edit.key), "edit called on %s which does not exist in the datastore.", edit.key.data());
					this->store[edit.key] = edit.val;
				},
				[this](detail::ds_remove remove)
				{
					tz::assert(this->contains(remove.key), "remove called on %s, which does not exist in the datastore", remove.key.data());
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
		return this->store.contains(key);
	}

	data_store_value data_store::read_raw(std::string_view key, bool dont_error_if_missing) const
	{
		std::shared_lock<mutex> slock(this->mtx);
		if(this->contains(key))
		{
			return this->store.at(key);
		}
		if(!dont_error_if_missing)
		{
			tz::error("`read_raw` called on %s, which does not exist in the datastore.", key.data());
		}
		return nullptr;
	}
}