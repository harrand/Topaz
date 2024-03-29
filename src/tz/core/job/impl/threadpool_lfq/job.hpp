#ifndef TZ_JOB_IMPL_THREADPOOL_LFQ_JOB_HPP
#define TZ_JOB_IMPL_THREADPOOL_LFQ_JOB_HPP
#include "tz/core/job/api/job.hpp"
#include "concurrentqueue.h"
#include <deque>
#include <thread>
#include <memory>
#include <mutex>
#include <condition_variable>

namespace tz::impl
{
	class job_system_threadpool_lfq : public i_job_system
	{
	public:
		job_system_threadpool_lfq();
		~job_system_threadpool_lfq();

		/// important note: if the job has an affinity, block(j) and complete(j) will not function correctly.
		virtual job_handle execute(job_t job, execution_info einfo = {}) override;
		virtual void block(job_handle j) const override;
		virtual bool complete(job_handle j) const override;
		virtual bool any_work_remaining() const override;
		virtual void block_all() const override;
		void new_frame();
		virtual std::size_t size() const override;
		virtual std::size_t worker_count() const override;
		virtual std::vector<worker_id_t> get_worker_ids() const override;
		unsigned int jobs_started_this_frame() const;
	private:
		struct job_info_t
		{
			job_t func;
			std::size_t id;
			std::optional<worker_id_t> affinity = std::nullopt;
		};
		struct worker_t
		{
			std::thread thread;
			std::size_t local_tid;
			std::atomic<std::size_t> current_job;
			moodycamel::ConcurrentQueue<job_info_t> affine_jobs;
		};
		void tmain(std::size_t local_tid);
		static void wait_a_bit();

		std::deque<worker_t> thread_pool;
		moodycamel::ConcurrentQueue<job_info_t> jobs;
		std::atomic<bool> requires_exit = false;
		std::atomic<std::size_t> lifetime_count = 0;
		std::atomic<std::size_t> count_this_frame = 0;
		std::vector<std::size_t> waiting_job_ids = {};
		mutable std::mutex waiting_job_id_mutex;
		mutable std::mutex wake_mutex;
		mutable std::condition_variable wake_condition;
	};
}

#endif // TZ_JOB_IMPL_THREADPOOL_LFQ_JOB_HPP
