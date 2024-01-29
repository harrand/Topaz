#include "tz/core/job/impl/concurrentqueue_blocking/job.hpp"
#include "tz/core/debug.hpp"
#include "tz/core/profile.hpp"
#include <atomic>
#include <limits>
#include <chrono>

#include <iostream>
#include <syncstream>

namespace tz::impl
{
	job_system_blockingcurrentqueue::job_system_blockingcurrentqueue():
	ptok(this->global_job_queue)
	{
		TZ_PROFZONE("job_system - initialise", 0xFFAA0000);
		this->running_job_ids.reserve(128);
		for(std::size_t i = 0; i < std::thread::hardware_concurrency() - 1; i++)
		{
			auto& worker = this->thread_pool.emplace_back();
			worker.thread = std::thread([this, i](){this->worker_thread_entrypoint(i);});
			worker.local_tid = i;
			worker.ctok = moodycamel::ConsumerToken{this->global_job_queue};
		}
	}

//--------------------------------------------------------------------------------------------------

	job_system_blockingcurrentqueue::~job_system_blockingcurrentqueue()
	{
		TZ_PROFZONE("job_system - terminate", 0xFFAA0000);
		this->close_requested.store(true);
		for(worker_t& worker : this->thread_pool)
		{
			worker.thread.join();
		}
		tz::assert(!this->any_work_remaining());
	}

//--------------------------------------------------------------------------------------------------

	job_handle job_system_blockingcurrentqueue::execute(job_t job, execution_info einfo)
	{
		TZ_PROFZONE("job_system - execute", 0xFFAA0000);
		job_handle ret
		{
			.handle = static_cast<hanval>(this->lifetime_jobs_created.load()), .owned = false
		};

		job_info_t jinfo
		{
			.func = job,
			.job_id = this->lifetime_jobs_created.load(),
			.maybe_affinity = einfo.maybe_worker_affinity	
		};
		this->lifetime_jobs_created.store(this->lifetime_jobs_created.load() + 1);
		this->jobs_created_this_frame.store(this->jobs_created_this_frame.load() + 1);
		{
			std::unique_lock<std::mutex> lock(this->done_job_list_mutex);
			this->running_job_ids.push_back(jinfo.job_id);
		}
		if(einfo.maybe_worker_affinity.has_value())
		{
			TZ_PROFZONE("execute - affine job enqueue", 0xFFAA8888);
			// add to list of affine jobs instead.
			auto val = einfo.maybe_worker_affinity.value();
			tz::assert(this->thread_pool[val].local_tid == val);
			this->thread_pool[val].affine_jobs.enqueue(jinfo);
			// ???dogshit?
			ret.owned = true;
		}
		else
		{
			TZ_PROFZONE("execute - global job enqueue", 0xFFAA8888);
			this->global_job_queue.enqueue(this->ptok, jinfo);
		}
		//std::osyncstream(std::cout) << "added new job " << jinfo.job_id << "\n";
		return ret;
	}

//--------------------------------------------------------------------------------------------------

	void job_system_blockingcurrentqueue::block(job_handle j) const
	{
		TZ_PROFZONE("job_system - block", 0xFFAA0000);
		using namespace std::chrono_literals;
		// while we dont think the job is done.
		//std::osyncstream(std::cout) << "blocking on job " << static_cast<std::size_t>(static_cast<tz::hanval>(j.handle)) << "\n";
		while(!this->complete(j))
		{
			// sleep time until any job is done.
			std::unique_lock<std::mutex> lock(this->wake_me_on_a_job_done_mutex);	
			this->wake_me_on_a_job_done.wait_for(lock, 1ms);
		}
		//std::osyncstream(std::cout) << "block on job " << static_cast<std::size_t>(static_cast<tz::hanval>(j.handle)) << " complete!\n";
	}

//--------------------------------------------------------------------------------------------------

	bool job_system_blockingcurrentqueue::complete(job_handle j) const
	{
		TZ_PROFZONE("job_system - is complete", 0xFFAA0000);
		// convert handle value into size_t (its actually the job id)
		auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(j.handle));
		// lock the done job list mutex.
		std::unique_lock<std::mutex> lock(this->done_job_list_mutex);
		// if it's not in the list of running jobs, then someone has done it.
		return std::find(this->running_job_ids.begin(), this->running_job_ids.end(), hanval) == this->running_job_ids.end();
	}

//--------------------------------------------------------------------------------------------------

	bool job_system_blockingcurrentqueue::any_work_remaining() const
	{
		TZ_PROFZONE("job_system - any work remaining", 0xFFAA0000);
		if(this->lifetime_jobs_created.load() == 0)
		{
			return false;
		}
		bool all_complete = true;
		std::unique_lock<std::mutex> lock(this->done_job_list_mutex);
		for(std::size_t i = 0; i < this->lifetime_jobs_created.load(); i++)
		{
			all_complete &= std::find(this->running_job_ids.begin(), this->running_job_ids.end(), i) == this->running_job_ids.end();
		}
		// worker_t stores the currently running job id.
		// so we could just check if all of them have no running job.
		// but that doesnt mean work isnt pending.
		// also global_job_queue.size_approx() == 0 is not necessarily accurate.
		// if we do both it might be kinda reasonable? not accurate still tho.
		bool any_workers_busy = false;
		for(const worker_t& worker : this->thread_pool)
		{
			if(worker.get_running_job().has_value())
			{
				any_workers_busy = true;
			}
		}
		auto approx_job_count = this->global_job_queue.size_approx();
		return approx_job_count > 0 || any_workers_busy || !all_complete;
	}

//--------------------------------------------------------------------------------------------------

	void job_system_blockingcurrentqueue::block_all() const
	{
		TZ_PROFZONE("job_system - block all", 0xFFAA0000);
		using namespace std::chrono_literals;
		//std::osyncstream(std::cout) << "waiting for all remaining jobs to complete...\n";
		// VERY similar to block, but for any_work_remaining instead of complete(j)
		while(this->any_work_remaining())
		{
			// sleep time until any job is done.
			std::unique_lock<std::mutex> lock(this->wake_me_on_a_job_done_mutex);	
			this->wake_me_on_a_job_done.wait_for(lock, 1ms);
		}
		//std::osyncstream(std::cout) << "no jobs still running!...\n";
	}

//--------------------------------------------------------------------------------------------------

	void job_system_blockingcurrentqueue::new_frame()
	{
		this->jobs_created_this_frame.store(0);
	}

//--------------------------------------------------------------------------------------------------

	std::size_t job_system_blockingcurrentqueue::size() const
	{
		return this->global_job_queue.size_approx();
	}

//--------------------------------------------------------------------------------------------------

	std::size_t job_system_blockingcurrentqueue::worker_count() const
	{
		return std::max(1u + static_cast<std::size_t>(this->thread_pool.size() * this->aggression), this->thread_pool.size());
	}

//--------------------------------------------------------------------------------------------------

	std::vector<worker_id_t> job_system_blockingcurrentqueue::get_worker_ids() const
	{
		std::vector<worker_id_t> ret;
		ret.reserve(this->worker_count());
		for(const auto& worker : this->thread_pool)
		{
			ret.push_back(worker.local_tid);
		}
		return ret;
	}

//--------------------------------------------------------------------------------------------------

	unsigned int job_system_blockingcurrentqueue::jobs_started_this_frame() const
	{
		return this->jobs_created_this_frame.load();
	}

//--------------------------------------------------------------------------------------------------

	float job_system_blockingcurrentqueue::get_aggression() const
	{
		return this->aggression.load();
	}
	
//--------------------------------------------------------------------------------------------------

	void job_system_blockingcurrentqueue::set_aggression(float aggression)
	{
		this->aggression.store(std::clamp(aggression, 0.0f, 1.0f), std::memory_order_relaxed);
	}

//--------------------------------------------------------------------------------------------------

	std::optional<std::size_t> job_system_blockingcurrentqueue::worker_t::get_running_job() const
	{
		std::size_t val = this->currently_running_job_id.load();
		if(val == std::numeric_limits<std::size_t>::max())
		{
			return std::nullopt;
		}
		return val;
	}

//--------------------------------------------------------------------------------------------------

	void job_system_blockingcurrentqueue::worker_thread_entrypoint(std::size_t local_tid)
	{
		std::string thread_name = "Topaz Job Thread " + std::to_string(local_tid);
		TZ_THREAD(thread_name.c_str());
		worker_t& worker = this->thread_pool[local_tid];
		constexpr std::int64_t queue_wait_timer_micros = 1000; // 1 millis. this is the minimum amount of time we're willing to wait if our spins yield nothing. in practice it will wait way longer.
		while(!this->close_requested.load())
		{
			job_info_t job;

			bool found = false;
			// some workers are disabled depending on the aggressiveness of the job system.
			// aggressiveness 0 means 1 worker.
			// aggressiveness 1.0 means all workers

			const float aggro = this->get_aggression();
			const float aggro_step = 1.0f / std::thread::hardware_concurrency();
			if(worker.local_tid > 0)
			{
				if(worker.local_tid > aggro / aggro_step)
				{
					// naptime. unless we have an affine job
					found = worker.affine_jobs.try_dequeue(job);
					if(!found)
					{
						//tz::report("worker %zu is naptime coz aggression is only %.2f", worker.local_tid, aggro);
						std::this_thread::sleep_for(std::chrono::milliseconds(5));
						continue;
					}
				}
			}

			// lets try to retrieve an affine job, if thats empty then get a job from the global queue.
			// this `if statement` could not happen if we hit the timeout without getting a job.
			// in which case we simply recurse.

			// note: on e.g windows sleeps suck asshole. 15-16ms is each quantum. moodycamel::concurrentqueue will spin for a maximum amount before actually sleeping - under which case we should expect to wait at least 16ms - too long.
			// what we really want to do is spin for a certain amount of time, not until a maximum number of spins.
			// however, how much we spin for *drastically* affects performance. spin for too short a time? you sleep almost instantly and kill perf when load is high
			// spin too long? you're maxing out cpu resources even when the application isnt doing anything.
			// the solution here is something PGO aligned.
			// im going to clamp between a very low spin time and a very high spin time, depending on how many jobs have been requested this frame.
			// this means if tons of jobs are requested, we spin for a long time to keep up.
			// if very few are submitted, we chill way the fuck out.

			// 10us is super tiny, will basically never catch anything.
			// 2000us is incredibly long. highly likely to catch everything. will also definitely max out the cpu usage.
			long long spin_duration = std::lerp(2, 3000, aggro);
			auto deadline = std::chrono::steady_clock::now() + std::chrono::microseconds(2000);
			{
				while(!found)
				{
					if(std::chrono::steady_clock::now() >= deadline)
					{
						// 10 micros of spinning has passed, and nothing. time to take the big bullet and sleep.
						TZ_PROFZONE("job worker - go to sleep", 0xFFAAFFEE);
						found = this->global_job_queue.wait_dequeue_timed(worker.ctok.value(), job, queue_wait_timer_micros);
						break;
					}
					found = worker.affine_jobs.try_dequeue(job) || this->global_job_queue.try_dequeue(worker.ctok.value(), job);
				}
			}
			if(found)
			{
				TZ_PROFZONE("job worker - do collected job", 0xFFAA0000);
				// we have a job to do
				worker.currently_running_job_id = job.job_id;
				//std::osyncstream(std::cout) << "[" << worker.local_tid << "] - running new job " << job.job_id << "\n";
				// do it
				{
					TZ_PROFZONE("job worker - run job function", 0xFFAA0000);
					job.func();
				}

				TZ_PROFZONE("job worker - mark job completed", 0xFFAA0000);
				// put it in the done list.
				{
					std::unique_lock<std::mutex> lock(this->done_job_list_mutex);
					// erase-remove idiom to remove this job id from list of running jobs.
					tz::assert(std::find(this->running_job_ids.begin(), this->running_job_ids.end(), job.job_id) != this->running_job_ids.end(), "Job that worker just completed was not in the list of running jobs!? Logic error.");
					this->running_job_ids.erase(std::remove(this->running_job_ids.begin(), this->running_job_ids.end(), job.job_id));
				}
				// wakey wakey on people waiting on a job to be done.
				//std::osyncstream(std::cout) << "[" << worker.local_tid << "] - finished job " << job.job_id << "\n";
				this->wake_me_on_a_job_done.notify_all();
				// and now we have no job again. recurse.
				worker.currently_running_job_id = std::numeric_limits<std::size_t>::max();
			}
		}
	}

}