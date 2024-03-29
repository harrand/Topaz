#include "tz/core/job/impl/threadpool_lfq/job.hpp"
#include "tz/core/debug.hpp"
#include "tz/core/profile.hpp"
#include <chrono>
#include <limits>
#include <functional>

namespace tz::impl
{
	constexpr std::size_t job_id_null = std::numeric_limits<std::size_t>::max();

	job_system_threadpool_lfq::job_system_threadpool_lfq()
	{
		TZ_PROFZONE("job_system_threadpool_lfq()", 0xFFAA0000);
		for(std::size_t i = 0; i < std::thread::hardware_concurrency(); i++)
		{
			auto& worker = this->thread_pool.emplace_back();
			worker.thread = std::thread([this, i](){this->tmain(i);});
			worker.local_tid = i;
			worker.current_job = job_id_null;
		}
	}
	
	job_system_threadpool_lfq::~job_system_threadpool_lfq()
	{
		TZ_PROFZONE("~job_system_threadpool_lfq()", 0xFFAA0000);
		{
			std::unique_lock<std::mutex> lock(this->wake_mutex);
			this->requires_exit = true;
		}
		this->wake_condition.notify_all();
		for(worker_t& worker : this->thread_pool)
		{
			worker.thread.join();
		}
		tz::assert(!this->any_work_remaining());
	}

	job_handle job_system_threadpool_lfq::execute(job_t job, execution_info einfo)
	{
		TZ_PROFZONE("job_system_threadpool_lfq::execute(job_t)", 0xFFAA0000);
		job_info_t jinfo
		{
			.func = job,
			.id = this->lifetime_count.load(),
			.affinity = einfo.maybe_worker_affinity
		};
		if(einfo.maybe_worker_affinity.has_value())
		{
			// get the worker of the necessary tid and then send it the job directly.
			auto val = einfo.maybe_worker_affinity.value();
			auto& q = this->thread_pool[val].affine_jobs;
			tz::assert(this->thread_pool[val].local_tid == val);
			std::size_t sz = q.size_approx();
			q.enqueue(jinfo);
			tz::assert(q.size_approx() > sz);
			this->count_this_frame++;
			// we need to guarantee the target thread wakes up, so we get them all (slow and bad booo)
			this->wake_condition.notify_all();
			return {.handle = static_cast<hanval>(sz), .owned = true};
		}
		{
			std::unique_lock<std::mutex> lock(this->waiting_job_id_mutex);
			this->waiting_job_ids.push_back(this->lifetime_count);
		}
		this->jobs.enqueue(jinfo);
		this->wake_condition.notify_one();
		this->count_this_frame++;
		return {.handle = static_cast<hanval>(this->lifetime_count++), .owned = false};
	}

	void job_system_threadpool_lfq::block(job_handle j) const
	{
		TZ_PROFZONE("job_system_threadpool_lfq::block(job_handle)", 0xFFAA0000);
		while(!this->complete(j))
		{
			job_system_threadpool_lfq::wait_a_bit();
		}
	}

	bool job_system_threadpool_lfq::complete(job_handle j) const
	{
		std::unique_lock<std::mutex> lock(this->waiting_job_id_mutex);
		return std::find(this->waiting_job_ids.begin(), this->waiting_job_ids.end(), static_cast<std::size_t>(static_cast<hanval>(j.handle))) == this->waiting_job_ids.end() && std::all_of(this->thread_pool.begin(), this->thread_pool.end(), [jid = static_cast<std::size_t>(static_cast<hanval>(j.handle))](const worker_t& worker){return worker.current_job != jid;});
	}

	bool job_system_threadpool_lfq::any_work_remaining() const
	{
		std::unique_lock<std::mutex> lock(this->waiting_job_id_mutex);
		return !this->waiting_job_ids.empty() || this->size() > 0 || std::any_of(this->thread_pool.begin(), this->thread_pool.end(), [](const worker_t& worker){return worker.current_job != job_id_null;});
	}

	void job_system_threadpool_lfq::block_all() const
	{
		TZ_PROFZONE("job_system_threadpool_lfq::block_all()", 0xFFAA0000);
		while(this->any_work_remaining())
		{
			job_system_threadpool_lfq::wait_a_bit();
		}
	}

	void job_system_threadpool_lfq::new_frame()
	{
		this->count_this_frame = 0;
	}

	std::size_t job_system_threadpool_lfq::size() const
	{
		return this->jobs.size_approx();
	}

	std::size_t job_system_threadpool_lfq::worker_count() const
	{
		return this->thread_pool.size();
	}

	std::vector<worker_id_t> job_system_threadpool_lfq::get_worker_ids() const
	{
		std::vector<worker_id_t> ret;
		for(const worker_t& worker : this->thread_pool)
		{
			ret.push_back(worker.local_tid);
		}
		return ret;
	}

	unsigned int job_system_threadpool_lfq::jobs_started_this_frame() const
	{
		return this->count_this_frame.load();
	}

	void job_system_threadpool_lfq::tmain(std::size_t local_tid)
	{
		TZ_THREAD("tz::job_system() Worker");
		worker_t& worker = this->thread_pool[local_tid];
		while(!this->requires_exit.load())
		{
			job_info_t job;
			// check if there are any jobs in the global list.
			{
				std::unique_lock<std::mutex> lock(this->wake_mutex);
				while(!this->requires_exit.load() && !this->jobs.try_dequeue(job))
				{
					// no jobs. check if there's anything set to our affinity and do that instead.
					if(worker.affine_jobs.try_dequeue(job))
					{
						// just do it, we're safe.
						//tz::report("performing affine job (worker tid %zu)", worker.local_tid);
						tz::assert(job.affinity.has_value() && job.affinity.value() == worker.local_tid);
						worker.current_job = job.id;
						job.func();
						worker.current_job = job_id_null;
					}
					this->wake_condition.wait(lock);
				}
			}
			if(this->requires_exit.load())
			{
				break;
			}
			worker.current_job = job.id;
			TZ_PROFZONE("job thread - do job", 0xFFAAAA00);
			{
				std::unique_lock<std::mutex> lock(this->waiting_job_id_mutex);
				auto iter = std::find(this->waiting_job_ids.begin(), this->waiting_job_ids.end(), job.id);
				tz::assert(iter != this->waiting_job_ids.end(), "Job system thread took a job, but it's id is not on the list of waiting jobs, as it should be.");
				this->waiting_job_ids.erase(iter);
			}
			job.func();
			worker.current_job = job_id_null;
		}
	}
	
	void job_system_threadpool_lfq::wait_a_bit()
	{
		std::this_thread::sleep_for(std::chrono::duration<int, std::micro>(10));
	}
}
