#ifndef TOPAZ_SCHEDULER_HPP
#define TOPAZ_SCHEDULER_HPP
#include "core/time.hpp"
#include <queue>

namespace tz
{
    /**
     * A generic scheduler which is meant to keep time and tick some special event.
     * @tparam TickType Structure representing some tick event. It should be implicitly convertible to bool depicting whether the tick is active or not.
     */
    template<typename TickType>
    class IScheduler
    {
    public:
        IScheduler() = default;
        virtual ~IScheduler() = default;
        /**
         * Drive the scheduler, telling it the number of milliseconds that have passed since the last update invocation.
         */
        virtual void update(float delta_millis) = 0;
        /**
         * Retrieve the current tick. It may or may not be active.
         */
        virtual TickType tick() const = 0;
    };

    /**
     * FixedUpdateTicks are active after a certain number of milliseconds has passed.
     */
    struct FixedUpdateTick
    {
        const float expected_tick_duration;
        float actual_tick_duration;
        bool tick;

        static FixedUpdateTick null_tick();
        bool is_null() const;
        operator bool() const;
    };

    /**
     * Scheduler to tick after a fixed-number of milliseconds have passed. Useful for fixed updates every x ms as opposed to updates every frame.
     */
    class FixedUpdateScheduler : public IScheduler<FixedUpdateTick>
    {
    public:
        FixedUpdateScheduler(float millis_per_tick);
        virtual void update(float delta_millis) override;
        virtual FixedUpdateTick tick() const override;
    private:
        float millis_count;
        float millis_per_tick;
        mutable std::queue<FixedUpdateTick> unprocessed_ticks;
    };
}

#endif // TOPAZ_SCHEDULER_HPP