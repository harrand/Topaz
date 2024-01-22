#ifndef TOPAZ_CORE_MESSAGING_HPP
#define TOPAZ_CORE_MESSAGING_HPP
#include "tz/core/types.hpp"
#include <vector>
#include <mutex>
#include <ranges>

namespace tz
{
	namespace detail
	{
		// base class for a non-thread-safe receiver.
		// lock returns empty lock type (i.e doesnt lock anything)
		struct no_lock
		{
		protected:
			using lock_t = std::unique_lock<std::mutex>;
			lock_t lock() const{return {};}
		};

		// base class for a thread-safe receiver.
		// lock returns a unique lock to the internal mutex.
		struct with_lock
		{
		protected:
			std::mutex mtx = {};	
			using lock_t = std::unique_lock<std::mutex>;
			lock_t lock() {return lock_t{this->mtx};}
		};
	}

	/**
	 * Conditionally thread-safe message receiver.
	 * @tparam M Message Type. Must satisfy `tz::message<M>`
	 * @tparam thread_safe Whether the object should lock an internal mutex. Defaults to true, but you might want to set this to false for a thread-local receiver, for example.
	 */
	template<tz::message M, bool thread_safe = true>
	class message_receiver : protected std::conditional_t<thread_safe, detail::with_lock, detail::no_lock>
	{
	public:
		/// Send a message to the receiver. Conditionally thread-safe.
		void send_message(M msg);
		/// Send multiple messages at once. Conditionally thread-safe.
		void send_messages(tz::range_of<M> auto range);
		/// Retrieve the number of messages in the receiver's queue. Conditionally thread-safe.
		std::size_t message_count() const;
	protected:
		// Processes all messages and then clears the list.
		void process_messages();
		/// Invoked when a message is sent. Your override can return false to drop the message. By default, no messages are dropped. Not thread safe.
		virtual bool on_send_message(const M& msg) {return true;};
		/// Invoked when a message is processed. You must override this. Not thread safe.
		/// @note If processing a message causes the receiver to send a new message to itself, this is safe, but will not be processed until the next invocation to `process_messages`.
		constexpr virtual void process_message(const M& msg) = 0;
	private:
		using base_t = std::conditional_t<thread_safe, detail::with_lock, detail::no_lock>;
		std::vector<M> messages = {};
	};
}

#include "tz/core/messaging.inl"

#endif // TOPAZ_CORE_MESSAGING_HPP
