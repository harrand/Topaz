#ifndef TZ_JOB_API_JOB_HPP
#define TZ_JOB_API_JOB_HPP
#include <functional>
#include "tz/core/data/handle.hpp"

namespace tz
{
	namespace detail
	{
		struct job_handle_tag{};
	}
	/**
	 * @ingroup tz_job
	 * Represents a job that has begun execution within a @ref job_system_type.
	 */
	using job_handle = tz::handle<detail::job_handle_tag>;

	/**
	 * @ingroup tz_job
	 * Represents a job. Jobs have no arguments and no return value.
	 */
	using job_t = std::function<void()>;

	/**
	 * @ingroup tz_job
	 * API for job system. See @ref tz::job_system() for usage.
	 */
	template<typename T>
	concept job_system_type = requires(T t, job_t job, job_handle jh)
	{
		/**
		 * Execute a new job asynchronously.
		 * @return Handle representing the created job.
		 */
		{t.execute(job)} -> std::same_as<job_handle>;
		/**
		 * Block the current thread until the specified job has completed its work.
		 */
		{t.block(jh)} -> std::same_as<void>;
		/**
		 * Query as to whether the specified job has completed its work.
		 * @return true if the job is complete, otherwise false.
		 */
		{t.complete(jh)} -> std::same_as<bool>;
		/**
		 * Query as to whether any of the previously-created jobs are still running.
		 * @return true if all jobs have completed their work, otherwise false.
		 */
		{t.any_work_remaining()} -> std::same_as<bool>;
		/**
		 * Block the current thread until *all* running jobs have completed their work.
		 */
		{t.block_all()} -> std::same_as<void>;
		/**
		 * Retrieve the number of running jobs that have not yet reached completion.
		 */
		{t.size()} -> std::same_as<unsigned int>;
	};
}

#endif // TZ_JOB_API_JOB_HPP
