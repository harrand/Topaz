namespace tz
{
	template<tz::message M>
	void message_receiver<M>::send_message(M msg)
	{
		if(!this->on_send_message(msg))
		{
			return;
		}
		std::unique_lock<std::mutex> lock(this->mtx);
		this->messages.push_back(msg);
	}

	template<tz::message M>
	void message_receiver<M>::send_messages(tz::range_of<M> auto range)
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
			std::unique_lock<std::mutex> lock(this->mtx);
			this->messages.insert(this->messages.end(), non_dropped_messages.begin(), non_dropped_messages.end());
		}
	}
	

	template<tz::message M>
	std::size_t message_receiver<M>::message_count() const
	{
		std::unique_lock<std::mutex> lock(this->mtx);
		return this->messages.size();
	}

	template<tz::message M>
	void message_receiver<M>::process_messages()
	{
		using message_container_t = decltype(this->messages);
		message_container_t msgs = {};
		// lock for as short a time as possible. move messages into our own buffer and then relinquish lock.
		{
			std::unique_lock<std::mutex> lock(this->mtx);
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
}
