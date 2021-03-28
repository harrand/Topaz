#include "core/scheduler.hpp"

namespace tz
{
    FixedUpdateTick FixedUpdateTick::null_tick()
    {
        return {-1.0f, -1.0f, false};
    }

    bool FixedUpdateTick::is_null() const
    {
        return !this->tick;
    }

    FixedUpdateTick::operator bool() const
    {
        return !this->is_null();
    }

    FixedUpdateScheduler::FixedUpdateScheduler(float millis_per_tick): IScheduler(), millis_count(0.0f), millis_per_tick(millis_per_tick), unprocessed_ticks(){}

    void FixedUpdateScheduler::update(float delta_millis)
    {
        this->millis_count += delta_millis;
        if(this->millis_count >= millis_per_tick)
        {
            this->unprocessed_ticks.push({this->millis_per_tick, this->millis_count, true});
            this->millis_count -= millis_per_tick;
        }
    }

    FixedUpdateTick FixedUpdateScheduler::tick() const
    {
        if(this->unprocessed_ticks.empty())
        {
            return FixedUpdateTick::null_tick();
        }
        FixedUpdateTick top =  this->unprocessed_ticks.front();
        this->unprocessed_ticks.pop();
        return top;
    }
}