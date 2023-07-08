#ifndef TZ_GL_API_SCHEDULE_HPP
#define TZ_GL_API_SCHEDULE_HPP
#include "tz/core/data/handle.hpp"
#include "tz/core/debug.hpp"
#include <vector>
#include <span>
#include <unordered_set>
#include <optional>

namespace tz::gl
{
	using eid_t = unsigned int;
	constexpr eid_t nullevt = std::numeric_limits<eid_t>::max();
	struct event
	{
		eid_t eid = nullevt;
		std::vector<eid_t> dependencies = {};
	};

	struct timeline_t : public std::vector<eid_t>
	{
		timeline_t() = default;
		template<typename... Is>
		timeline_t(Is&&... is)
		{
			(this->push_back(static_cast<eid_t>(static_cast<tz::hanval>(is))), ...);
		}
	};

	struct schedule
	{
		std::vector<event> events = {};
		timeline_t timeline = {};

		std::span<const eid_t> get_dependencies(eid_t evt) const;

		template<typename T0, typename... T>
		void add_dependencies(T0 evth, T... deps)
		{
			auto evt = static_cast<eid_t>(static_cast<tz::hanval>(evth));
			auto iter = std::find_if(this->events.begin(), this->events.end(), [evt](const auto& event){return event.eid == evt;});
			tz::assert(iter != this->events.end());
			(iter->dependencies.push_back(static_cast<eid_t>(static_cast<tz::hanval>(deps))), ...);
		}

		template<typename H>
		unsigned int chronological_rank(H handle) const
		{
			auto evt = static_cast<eid_t>(static_cast<tz::hanval>(handle));
			return chronological_rank_eid(evt);
		}

		unsigned int max_chronological_rank() const
		{
			unsigned int maxrank = 0;
			for(eid_t evt : this->timeline)
			{
				maxrank = std::max(maxrank, this->chronological_rank_eid(evt));
			}
			return maxrank;
		}

		std::optional<unsigned int> get_wait_rank(eid_t evt) const
		{
			if(this->chronological_rank_eid(evt) == 0)
			{
				return std::nullopt;
			}
			unsigned int max_wait_rank = 0;
			for(eid_t dep : this->get_dependencies(evt))
			{
				max_wait_rank = std::max(max_wait_rank, this->chronological_rank_eid(dep));
			}
			return {max_wait_rank};
		}

		unsigned int chronological_rank_eid(eid_t evt) const
		{
			unsigned int rank = 0;
			if(this->get_dependencies(evt).size())
			{
				for(std::size_t dep : this->get_dependencies(evt))
				{
					rank = std::max(rank, this->chronological_rank_eid(dep));
				}
				rank++;
			}
			return rank;
		}
	};

}

#endif // TZ_GL_API_SCHEDULE_HPP
