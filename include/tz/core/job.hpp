#ifndef TOPAZ_CORE_JOB_HPP
#define TOPAZ_CORE_JOB_HPP
#include "tz/core/handle.hpp"
#include <functional>

namespace tz
{
	namespace detail
	{
		struct job_tag_t{};
	}
	using job_handle = tz::handle<detail::job_tag_t>;
	/**
	 * @ingroup tz_core
	 * @brief Represents a function that will be executed on a job worker via @ref job_execute.
	 */
	using job_function = std::function<void()>;
	using job_worker = std::size_t;

	/**
	 * @ingroup tz_core
	 * @brief Execute a function as a new job.
	 *
	 * This creates a new job which will be picked up by any worker thread as soon as possible. You can query the job's progress via @ref job_complete, or block the wait for it to finish via @ref job_wait.
	 *
	 * There is no guarantee or hint as to which worker thread ultimately picks up your work. If you need that, consider @ref job_execute_on. There is also no safety mechanism in place for avoiding deadlocks -- there is nothing stopping you from writing race conditions, nor does some hidden feature exist to protect you from them.
	 *
	 * - It is safe to execute new jobs within another running job.
	 * - There is no way to cancel a job, you can only wait for it to finish.
	 */
	job_handle job_execute(job_function fn);
	/**
	 * @ingroup tz_core
	 * @brief Execute a function as a new job - but can only be picked up by a specific worker.
	 *
	 * @note As a rule-of-thumb, prefer @ref job_execute in all situations unless you *must* run work on a specific thread.
	 *
	 * At any given time, there are a set of worker threads available for use (number of worker threads is equal to @ref job_worker_count). Unlike @ref job_execute, the job created by this function is tailored to only a single worker thread. Aside from this, the behaviour of this function exactly matches that of @ref job_execute.
	 *
	 * You might want to do this for a couple of reasons:
	 * - You know a specific worker thread has a certain state, which the job needs access to.
	 * - You wish to execute exactly *one* job on *every* worker thread, for whatever reason.
	 * - You are delusional and think you can do a better job than your OS's scheduler.
	 */
	job_handle job_execute_on(job_function fn, job_worker worker);
	/**
	 * @ingroup tz_core
	 * @brief Block the current thread until the job specified has been fully completed.
	 */
	void job_wait(job_handle job);
	/**
	 * @ingroup tz_core
	 * @brief Query as to whether the specific job has been fully completed or not.
	 */
	bool job_complete(job_handle job);
	/**
	 * @ingroup tz_core
	 * @brief Estimate the total number of jobs that have been created but not yet completed.
	 *
	 * You should assume the returned value to be an estimate -- a lower bound on the real number. It is also absolutely possible that many jobs are completed between the time of you calling this function and inspecting the result.
	 */
	std::size_t job_count();
	/**
	 * @ingroup tz_core
	 * @brief Retrieve the number of worker threads.
	 *
	 * The returned value is unaffected by whether these worker threads are currently carrying out work/are idle. You can assume this number will never change throughout your application's runtime.
	 */
	std::size_t job_worker_count();
}

#endif // TOPAZ_CORE_JOB_HPP