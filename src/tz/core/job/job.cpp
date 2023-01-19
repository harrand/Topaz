#include "tz/core/job/job.hpp"
#include "tz/core/debug.hpp"
#include <memory>

namespace tz
{
	std::unique_ptr<job_system_t> sys = nullptr;

	namespace detail
	{
		void job_system_init()
		{
			sys = std::make_unique<job_system_t>();
		}

		void job_system_term()
		{
			sys = nullptr;
		}
	}

	job_system_t& job_system()
	{
		tz::assert(sys != nullptr);
		return *sys;
	}
}
