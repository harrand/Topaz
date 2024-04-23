#ifndef TOPAZ_CORE_MESSAGING_HPP
#define TOPAZ_CORE_MESSAGING_HPP
#include "tz/core/types.hpp"
#include "tz/core/profile.hpp"
#include <vector>
#include <mutex>
#include <ranges>
#include <variant>

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
			mutable std::mutex mtx = {};	
			using lock_t = std::unique_lock<std::mutex>;
			lock_t lock() const
			{
				TZ_PROFZONE("message receiver - lock", 0xFFCCAACC);
				return lock_t{this->mtx};
			}
		};
	}

	/**
	 * @ingroup tz_core_messaging
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
		// Processes all messages and then clears the list. Thread-safe.
		virtual void process_messages();
		/// Invoked when a message is sent. Your override can return false to drop the message. By default, no messages are dropped. Not thread safe.
		virtual bool on_send_message([[maybe_unused]] const M& msg) {return true;};
		/// Invoked when a message is processed. You must override this. Not thread safe.
		/// @note If processing a message causes the receiver to send a new message to itself, this is safe, but will not be processed until the next invocation to `process_messages`.
		virtual void process_message(const M& msg) = 0;
	protected:
		using base_t = std::conditional_t<thread_safe, detail::with_lock, detail::no_lock>;
		std::vector<M> messages = {};
	};

	/**
	 * @ingroup tz_core_messaging
	 * Conditionally thread-safe message passer.
	 *
	 * When you send messages to a passer, they are held in the queue just like a normal passer. However, when a passer processes its messages, it simply re-sends those messages to its target receiver.
	 *
	 * If the passer has no target, message processing does nothing.
	 */ 
	template<tz::message M, bool thread_safe = true>
	class message_passer : public message_receiver<M, thread_safe>
	{
	public:
		/// Create a passer with no target.
		message_passer() = default;
		/// Create a passer with an existing target.
		message_passer(message_receiver<M, false>& target);
		/// Create a passer with an existing target.
		message_passer(message_receiver<M, true>& target);
		/// Redirect all messages to the target.
		virtual void process_messages() override final;
		/// Redirect messages to the target.
		virtual void process_message(const M& msg) override final;

		/// Redirect messages to a new target. Not thread-safe.
		void set_target(message_receiver<M, true>& target);
		/// Redirect messages to a new target. Not thread-safe.
		void set_target(message_receiver<M, false>& target);
		/// Clear the target. Processed messages will no longer be redirected. Not thread-safe.
		void clear_target();
	private:
		using base_t = message_receiver<M, thread_safe>::base_t;
		std::variant<message_receiver<M, true>*, message_receiver<M, false>*, std::monostate> target = std::monostate{};
	};
}

#include "tz/core/messaging.inl"

#endif // TOPAZ_CORE_MESSAGING_HPP
