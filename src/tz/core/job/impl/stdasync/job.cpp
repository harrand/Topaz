#include "tz/core/job/impl/stdasync/job.hpp"
#include <thread>
#include <chrono>

namespace tz::impl
{
	namespace detail
	{
		bool is_ready(const std::future<void>& f)
		{
			return f.valid() && f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
		}
	}

	job_system_stdasync::job_system_stdasync()
	{
	}

	job_handle job_system_stdasync::execute(job_t job)
	{
		auto j = this->get_unused_future();
		if(!j.has_value())
		{
			this->jobs.push_back(std::async(std::launch::async, [job](){job();}));
			return static_cast<hanval>(this->jobs.size() - 1);
		}
		else
		{
			auto jv = j.value();
			this->jobs[jv] = std::async(std::launch::async, [job](){job();});
			return static_cast<hanval>(jv);
		}
	}

	void job_system_stdasync::block(job_handle j) const
	{
		if(this->complete(j))
		{
			return;
		}
		return this->jobs[static_cast<std::size_t>(static_cast<hanval>(j))].wait();
	}

	bool job_system_stdasync::complete(job_handle j) const
	{
		return detail::is_ready(this->jobs[static_cast<std::size_t>(static_cast<hanval>(j))]);
	}

	bool job_system_stdasync::any_work_remaining() const
	{
		return std::any_of(this->jobs.begin(), this->jobs.end(), [](const std::future<void>& future){return !detail::is_ready(future);});
	}

	void job_system_stdasync::block_all() const
	{
		for(std::size_t i = 0; i < this->size(); i++)
		{
			job_handle h = static_cast<hanval>(i);
			this->block(h);
		}
	}

	unsigned int job_system_stdasync::size() const
	{
		return this->jobs.size();
	}

	std::optional<std::size_t> job_system_stdasync::get_unused_future()
	{
		for(std::size_t i = 0; i < this->jobs.size(); i++)
		{
			if(!this->jobs[i].valid() || detail::is_ready(this->jobs[i]))
			{
				return i;
			}
		}
		return std::nullopt;
	}
}
