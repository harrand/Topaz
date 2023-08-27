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
	 * @ingroup tz_core_job
	 * Represents a job that has begun execution within a @ref job_system_type.
	 */
	using job_handle = tz::handle<detail::job_handle_tag>;

	/**
	 * @ingroup tz_core_job
	 * Represents a job. Jobs have no arguments and no return value.
	 */
	using job_t = std::function<void()>;

	class i_job_system
	{
	public:
		virtual ~i_job_system() = default;
		virtual job_handle execute(job_t job) = 0;
		virtual void block(job_handle jh) const = 0;
		virtual void block_all() const = 0;
		virtual bool complete(job_handle jh) const = 0;
		virtual std::size_t size() const = 0;
		virtual bool any_work_remaining() const = 0;
	};
}

#endif // TZ_JOB_API_JOB_HPP
