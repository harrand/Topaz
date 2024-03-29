#include "tz/core/debug.hpp"
namespace tz
{
	template<tz::message M, bool thread_safe>
	void message_receiver<M, thread_safe>::send_message(M msg)
	{
		if(!this->on_send_message(msg))
		{
			return;
		}

		typename base_t::lock_t lock = base_t::lock();
	
		this->messages.push_back(msg);
	}

	template<tz::message M, bool thread_safe>
	void message_receiver<M, thread_safe>::send_messages(tz::range_of<M> auto range)
	{
		std::vector<M> non_dropped_messages;
		non_dropped_messages.reserve(std::ranges::size(range));
		for(const M& msg : range)
		{
			if(this->on_send_message(msg))
			{
				non_dropped_messages.push_back(msg);
			}
		}
		if(non_dropped_messages.size())
		{
			typename base_t::lock_t lock = base_t::lock();
			this->messages.insert(this->messages.end(), non_dropped_messages.begin(), non_dropped_messages.end());
		}
	}
	

	template<tz::message M, bool thread_safe>
	std::size_t message_receiver<M, thread_safe>::message_count() const
	{
		typename base_t::lock_t lock = base_t::lock();
		return this->messages.size();
	}

	template<tz::message M, bool thread_safe>
	void message_receiver<M, thread_safe>::process_messages()
	{
		using message_container_t = decltype(this->messages);
		message_container_t msgs = {};
		// lock for as short a time as possible. move messages into our own buffer and then relinquish lock.
		{
			typename base_t::lock_t lock = base_t::lock();
			msgs = std::move(this->messages);
			tz::assert(this->messages.empty());
			//this->messages.clear();
		}
		// iterate and process.	
		for(const M& msg : msgs)
		{
			this->process_message(msg);
		}
	}

	template<tz::message M, bool thread_safe>
	message_passer<M, thread_safe>::message_passer(message_receiver<M, true>& target):
	target(&target)
	{}

	template<tz::message M, bool thread_safe>
	message_passer<M, thread_safe>::message_passer(message_receiver<M, false>& target):
	target(&target)
	{}

	template<tz::message M, bool thread_safe>
	void message_passer<M, thread_safe>::process_messages()
	{
		typename base_t::lock_t lock = base_t::lock();
		if(this->messages.empty())
		{
			return;
		}
		std::visit([this](auto&& receiver_ptr)
		{
			if constexpr(!std::is_same_v<std::decay_t<decltype(receiver_ptr)>, std::monostate>)
			{
				if(receiver_ptr != nullptr)
				{
					receiver_ptr->send_messages(this->messages);
				}
			}
		}, this->target);
		this->messages.clear();
	}

	template<tz::message M, bool thread_safe>
	void message_passer<M, thread_safe>::process_message(const M& msg)
	{
		std::visit([&msg](auto&& receiver_ptr)
		{
			if constexpr(!std::is_same_v<std::decay_t<decltype(receiver_ptr)>, std::monostate>)
			{
				if(receiver_ptr != nullptr)
				{
					receiver_ptr->send_message(msg);
				}
			}
		}, this->target);
	}

	template<tz::message M, bool thread_safe>
	void message_passer<M, thread_safe>::set_target(message_receiver<M, true>& target)
	{
		this->target = &target;
	}

	template<tz::message M, bool thread_safe>
	void message_passer<M, thread_safe>::set_target(message_receiver<M, false>& target)
	{
		this->target = &target;
	}

	template<tz::message M, bool thread_safe>
	void message_passer<M, thread_safe>::clear_target()
	{
		this->target = std::monostate{};
	}
}
