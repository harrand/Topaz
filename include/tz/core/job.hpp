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
	using job_function = std::function<void()>;
	using job_worker = std::size_t;

	job_handle job_execute(job_function fn);
	job_handle job_execute_on(job_function fn, job_worker worker);
	void job_wait(job_handle job);
	void job_wait_all();
	bool job_complete(job_handle job);
	std::size_t job_count();
	std::size_t job_worker_count();
}

#endif // TOPAZ_CORE_JOB_HPP