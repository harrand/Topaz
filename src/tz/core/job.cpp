#include "tz/core/job.hpp"
#include "concurrentqueue.h"
#include "tz/detail/debug.hpp"
#include <thread>
#include <deque>
#include <optional>
#include <mutex>
#include <atomic>
#include <chrono>

namespace tz
{
	// state begin

	struct job_data
	{
		job_function fn;
		std::size_t job_id;
		std::optional<job_worker> affinity = std::nullopt;
	};

	struct worker_data
	{
		std::thread thread;
		job_worker my_id;
		std::atomic<std::size_t> current_job;
		moodycamel::ConcurrentQueue<job_data> affine_jobs;
	};

	std::deque<worker_data> workers;
	moodycamel::ConcurrentQueue<job_data> jobs;
	std::atomic<bool> requires_exit = false;
	std::atomic<std::size_t> lifetime_count = 0;
	std::vector<std::size_t> waiting_job_ids = {};
	std::mutex waiting_job_id_mutex;
	std::mutex wake_mutex;
	std::condition_variable wake_condition;

	// state end, api begin

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

	// api end, init/term
	constexpr std::size_t job_id_null = std::numeric_limits<std::size_t>::max();

	void impl_wait()
	{
		std::this_thread::sleep_for(std::chrono::duration<int, std::micro>(10));
	}

	void impl_tmain(std::size_t tid)
	{
		worker_data& me = workers[tid];
		while(!requires_exit.load())
		{
			job_data job;
			{
				std::unique_lock<std::mutex> lock(wake_mutex);
				while(!requires_exit.load() && !jobs.try_dequeue(job))
				{
					// no jobs. how about affine jobs?
					if(me.affine_jobs.try_dequeue(job))
					{
						// we have an affine job.
						me.current_job = job.job_id;
						job.fn();
						me.current_job = job_id_null;
					}
					wake_condition.wait(lock);
				}
			}
			if(requires_exit.load())
			{
				break;
			}
			me.current_job = job.job_id;
			{
				std::unique_lock<std::mutex> lock(waiting_job_id_mutex);
				auto iter = std::find(waiting_job_ids.begin(), waiting_job_ids.end(), job.job_id);
				tz_assert(iter != waiting_job_ids.end(), "job system thread took a job, but its id is not on the list of waiting jobs???");
			}
			job.fn();
			me.current_job = job_id_null;
		}
	}

	namespace detail
	{
		void job_system_initialise()
		{
			for(std::size_t i = 0; i < job_worker_count(); i++)
			{
				auto& worker = workers.emplace_back();
				worker.thread = std::thread([i](){impl_tmain(i);});
				worker.my_id = i;
				worker.current_job = job_id_null;
			}
		}

		void job_system_terminate()
		{
			{
				std::unique_lock<std::mutex> lock(wake_mutex);
				requires_exit = true;
			}
			wake_condition.notify_all();
			for(worker_data& worker : workers)
			{
				worker.thread.join();
			}
		}
	}

}