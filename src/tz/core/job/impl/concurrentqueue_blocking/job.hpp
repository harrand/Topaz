#ifndef TZ_JOB_IMPL_CONCURRENTQUEUE_BLOCKING_JOB_HPP
#define TZ_JOB_IMPL_CONCURRENTQUEUE_BLOCKING_JOB_HPP
#include "tz/core/job/api/job.hpp"
#include "blockingconcurrentqueue.h"
#include <deque>
#include <mutex>
#include <condition_variable>

namespace tz::impl
{
	class job_system_blockingcurrentqueue : public i_job_system
	{
	public:
		job_system_blockingcurrentqueue();
		~job_system_blockingcurrentqueue();

		virtual job_handle execute(job_t job, execution_info einfo = {}) override;
		virtual void block(job_handle j) const override;
		virtual bool complete(job_handle j) const override;
		virtual bool any_work_remaining() const override;
		virtual void block_all() const override;
		void new_frame(); // ???
		virtual std::size_t size() const override;
		virtual std::size_t worker_count() const override;
		virtual std::vector<worker_id_t> get_worker_ids() const override;
		unsigned int jobs_started_this_frame() const;
	private:
		struct job_info_t
		{
			job_t func;
			std::size_t job_id;
			std::optional<worker_id_t> maybe_affinity = std::nullopt;
		};

		struct worker_t
		{
			std::thread thread;
			std::size_t local_tid;
			std::atomic<std::size_t> currently_running_job_id = std::numeric_limits<std::size_t>::max();
			moodycamel::ConcurrentQueue<job_info_t> affine_jobs;
			std::vector<std::size_t> completed_job_cache;
			std::optional<std::size_t> get_running_job() const;
			void clear_job_cache(job_system_blockingcurrentqueue& js);
		};
		friend struct worker_t;

		void worker_thread_entrypoint(std::size_t local_tid);

		std::deque<worker_t> thread_pool;
		moodycamel::BlockingConcurrentQueue<job_info_t> global_job_queue;
		mutable std::mutex done_job_list_mutex;
		std::vector<std::size_t> running_job_ids = {};
		mutable std::mutex wake_me_on_a_job_done_mutex;
		mutable std::condition_variable wake_me_on_a_job_done;
		std::atomic<std::uint64_t> lifetime_jobs_created = 0u;
		std::atomic<std::size_t> jobs_created_this_frame = 0u;
		std::atomic<bool> close_requested = false;
	};
}

#endif // TZ_JOB_IMPL_CONCURRENTQUEUE_BLOCKING_JOB_HPP