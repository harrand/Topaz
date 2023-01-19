#ifndef TZ_JOB_IMPL_STDASYNC_JOB_HPP
#define TZ_JOB_IMPL_STDASYNC_JOB_HPP
#include "tz/core/job/api/job.hpp"
#include <vector>
#include <future>
#include <optional>

namespace tz::impl
{
	class job_system_stdasync
	{
	public:
		job_system_stdasync();

		job_handle execute(job_t job);
		void block(job_handle j) const;
		bool complete(job_handle j) const;
		bool any_work_remaining() const;
		void block_all() const;
		unsigned int size() const;
	private:
		std::optional<std::size_t> get_unused_future();

		std::vector<std::future<void>> jobs;
	};
	static_assert(job_system_type<job_system_stdasync>);
}

#endif // TZ_JOB_IMPL_STDASYNC_JOB_HP
