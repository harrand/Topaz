#include "tz/lua/slab.hpp"
#include "tz/core/debug.hpp"
#include "tz/core/job/job.hpp"
#include "tz/lua/state.hpp"
#include <memory>
#include <utility>

namespace tz::lua
{
	void slab::clear()
	{
		this->operations.clear();
	}

	std::size_t slab::size() const
	{
		return this->operations.size();
	}

	std::span<const lua_operation> slab::data() const
	{
		return this->operations;
	}

	thread_local slab this_slab = {};
	thread_local std::unique_ptr<i_slab_processor> this_proc = nullptr;

	slab& get_slab()
	{
		return this_slab;
	}
	
	i_slab_processor& get_slab_processor()
	{
		if(this_proc == nullptr)
		{
			this_proc = std::make_unique<empty_slab_processor>();
			tz::assert(!this_proc->does_anything());
		}
		return *this_proc;
	}

	void install_slab_processor(i_slab_processor& proc)
	{
	   get_slab_processor() = proc; 
	}

	std::vector<lua_operation> process_single_slab(const slab& s)
	{
		// return a set of merged lua operations for the provided slab.
		auto& proc = get_slab_processor();
		// firstly, find all operations with the same subject and context
		using matched_context = std::pair<unsigned int, unsigned int>;

		struct matched_context_hash
		{
			std::size_t operator()(const matched_context& ctx) const
			{
				return std::hash<unsigned int>{}(ctx.first) ^ std::hash<unsigned int>{}(ctx.second);
			}
		};

		std::unordered_map<matched_context, std::vector<lua_operation>, matched_context_hash> sorted_operations;
		for(std::size_t i = 0; i < s.size(); i++)
		{
			for(std::size_t j = i; j < s.size(); j++)
			{
				const auto& left = s.data()[i];
				const auto& right = s.data()[i];
				if(left.context_id == right.context_id && left.subject_id == right.subject_id)
				{
					auto& list = sorted_operations[{left.context_id, left.subject_id}];
					list.push_back(left);
					list.push_back(right);
				}
			}
		}

		// secondly, for all operations sharing the same subject and context, merge them all together into one.
		std::vector<lua_operation> final_operations;
		final_operations.reserve(this_slab.size());
		for(auto& [ctx, list] : sorted_operations)
		{
			if(list.size())
			{
				// merge into a single op.
				lua_operation op = list.front();
				if(list.size() > 1)
				{
					for(std::size_t i = 1; i < list.size(); i++)
					{
						op = proc.merge(op, this_slab.data()[i]);
					}
				}
				// push that op into the final list.
				final_operations.push_back(op);
			}
		}
		return final_operations;
	}

	std::vector<lua_operation> process_slab()
	{
		// return a set of merged lua operations fo rthis thread's slab.
		return process_single_slab(this_slab);
	}

	void process_all_slabs()
	{
		// for all slabs on all worker threads, combine them into a single mega-list of lua operations, and then process them all.
		auto thread_count = tz::job_system().worker_count() + 1; // include this thread (main thread).
		slab final_slab;
		std::vector<std::vector<lua_operation>> all_operations;
		all_operations.resize(thread_count);
		std::atomic<int> counter = 0;
		// collaborate all final operations from all workers.
		tz::lua::for_all_states([&counter, &all_operations](tz::lua::state& state)
		{
			all_operations[counter++] = process_slab();
			this_slab.clear();
		});
		tz::assert(std::cmp_equal(counter.load(), thread_count));
		// combine them all. one more time.
		std::vector<lua_operation> combined_ops = process_single_slab(final_slab);
		for(lua_operation op : combined_ops)
		{
			this_proc->process_operation(op);
		}
	}
}
