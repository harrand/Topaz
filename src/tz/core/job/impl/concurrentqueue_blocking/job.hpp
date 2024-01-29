#ifndef TZ_JOB_IMPL_CONCURRENTQUEUE_BLOCKING_JOB_HPP
#define TZ_JOB_IMPL_CONCURRENTQUEUE_BLOCKING_JOB_HPP
#include "tz/core/job/api/job.hpp"
#include "blockingconcurrentqueue.h"
#include <deque>
#include <mutex>
#include <condition_variable>

namespace tz::impl
{
	struct concurrentqueue_traits : public moodycamel::ConcurrentQueueDefaultTraits
	{
		// here im trying to trade some ram for less churn on enqueue - too often enqueue takes a really long time (like over 1ms)
		static const size_t BLOCK_SIZE = 512;
		// given we expect a not-so-volatile number of new jobs to be enqueued every frame, recycling hopefully makes nice gains.
		static const bool RECYCLE_ALLOCATED_BLOCKS = true;
	};
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
		float get_aggression() const;
		void set_aggression(float aggression);
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
			std::optional<moodycamel::ConsumerToken> ctok = std::nullopt;
			std::vector<std::size_t> completed_job_cache;
			std::optional<std::size_t> get_running_job() const;
			void clear_job_cache(job_system_blockingcurrentqueue& js);
		};
		friend struct worker_t;

		void worker_thread_entrypoint(std::size_t local_tid);

		std::deque<worker_t> thread_pool;
		moodycamel::BlockingConcurrentQueue<job_info_t, concurrentqueue_traits> global_job_queue;
		moodycamel::ProducerToken ptok;
		mutable std::mutex done_job_list_mutex;
		std::vector<std::size_t> running_job_ids = {};
		mutable std::mutex wake_me_on_a_job_done_mutex;
		mutable std::condition_variable wake_me_on_a_job_done;
		std::atomic<std::uint64_t> lifetime_jobs_created = 0u;
		std::atomic<std::size_t> jobs_created_this_frame = 0u;
		std::atomic<bool> close_requested = false;
		std::atomic<float> aggression = 0.0f;
	};
}

#endif // TZ_JOB_IMPL_CONCURRENTQUEUE_BLOCKING_JOB_HPP