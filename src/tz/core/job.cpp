#include "tz/core/job.hpp"
#include "concurrentqueue.h"
#include <thread>

namespace tz
{
	job_handle job_execute(job_function fn)
	{
		(void)fn;
		return tz::nullhand;
	}

	job_handle job_execute_on(job_function fn, job_worker worker)
	{
		(void)fn;
		(void)worker;
		return tz::nullhand;
	}

	void job_wait(job_handle job)
	{
		(void)job;
	}

	void job_wait_all()
	{

	}

	bool job_complete(job_handle job)
	{
		(void)job;
		return false;
	}

	std::size_t job_count()
	{
		return 0;
	}

	std::size_t job_worker_count()
	{
		return std::thread::hardware_concurrency();
	}

	// init/term

	namespace detail
	{
		void job_system_initialise()
		{

		}

		void job_system_terminate()
		{

		}
	}

}