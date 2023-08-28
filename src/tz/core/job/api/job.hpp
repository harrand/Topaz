#ifndef TZ_JOB_API_JOB_HPP
#define TZ_JOB_API_JOB_HPP
#include <functional>
#include <optional>
#include "tz/core/data/handle.hpp"

namespace tz
{
	namespace detail
	{
		struct job_handle_tag{};
	}
	/**
	 * @ingroup tz_core_job
	 * Represents a job that has begun execution within a @ref job_system_type.
	 */
	using job_handle_data = tz::handle<detail::job_handle_tag>;
	struct job_handle
	{
		job_handle_data handle;
		bool owned = false;
	};

	/**
	 * @ingroup tz_core_job
	 * Represents a job. Jobs have no arguments and no return value.
	 */
	using job_t = std::function<void()>;

	using worker_id_t = std::size_t;

	/**
	 * @ingroup tz_core_job
	 * Additional optional data about how a job is to be executed.
	 * You most likely don't want to change anything, unless you *really* know what you're doing.
	 */
	struct execution_info
	{
		/// Assign an affinity to the job execution, meaning that only the worker with the thread id matching the value will work on it. @note This drastically increases contention and thus hurts performance, don't do this unless absolutely necessary.
		std::optional<worker_id_t> maybe_worker_affinity = std::nullopt;
	};

	class i_job_system
	{
	public:
		virtual ~i_job_system() = default;
		virtual job_handle execute(job_t job, execution_info einfo = {}) = 0;
		virtual void block(job_handle jh) const = 0;
		virtual void block_all() const = 0;
		virtual bool complete(job_handle jh) const = 0;
		virtual std::size_t size() const = 0;
		virtual std::size_t worker_count() const = 0;
		virtual std::vector<worker_id_t> get_worker_ids() const = 0;
		virtual bool any_work_remaining() const = 0;
	};
}

#endif // TZ_JOB_API_JOB_HPP
