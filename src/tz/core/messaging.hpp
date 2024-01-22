#ifndef TOPAZ_CORE_MESSAGING_HPP
#define TOPAZ_CORE_MESSAGING_HPP
#include "tz/core/types.hpp"
#include <vector>
#include <mutex>
#include <ranges>

namespace tz
{
	template<tz::message M>
	class message_receiver
	{
	public:
		/// Send a message to the receiver. Thread-safe.
		void send_message(M msg);
		/// Send multiple messages at once. Thread-safe.
		void send_messages(tz::range_of<M> auto range);
		/// Retrieve the number of messages in the receiver's queue. Thread-safe.
		std::size_t message_count() const;
	protected:
		// Processes all messages and then clears the list.
		void process_messages();
		/// Invoked when a message is sent. Your override can return false to drop the message. By default, no messages are dropped.
		virtual bool on_send_message(const M& msg) {return true;};
		/// Invoked when a message is processed. You must override this.
		/// @note If processing a message causes the receiver to send a new message to itself, this is safe, but will not be processed until the next invocation to `process_messages`.
		constexpr virtual void process_message(const M& msg) = 0;

		std::vector<M> messages = {};
		std::mutex mtx = {};
	};
}

#include "tz/core/messaging.inl"

#endif // TOPAZ_CORE_MESSAGING_HPP
