#ifndef TZ_JOB_JOB_HPP
#define TZ_JOB_JOB_HPP

// TODO: Configurable
#include "tz/core/job/impl/stdasync/job.hpp"
#include "tz/core/job/impl/threadpool_lfq/job.hpp"
#undef assert

namespace tz
{
	/**
	 * @ingroup tz_job
	 * Underlying job system. See @ref tz::job_system_type for API.
	 */
	using job_system_t = tz::impl::job_system_threadpool_lfq;

	namespace detail
	{
		void job_system_init();
		void job_system_term();
	}

	/**
	 * @ingroup tz_job
	 * Retrieve the global job system.
	 */
	job_system_t& job_system();
}

#endif // TZ_JOB_JOB_HPP
