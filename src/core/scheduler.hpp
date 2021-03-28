#ifndef TOPAZ_SCHEDULER_HPP
#define TOPAZ_SCHEDULER_HPP
#include "core/time.hpp"
#include <queue>

namespace tz
{
    template<typename TickType>
    class IScheduler
    {
    public:
        IScheduler() = default;
        virtual ~IScheduler() = default;
        virtual void update(float delta_millis) = 0;
        virtual TickType tick() const = 0;
    };

    struct FixedUpdateTick
    {
        const float expected_tick_duration;
        float actual_tick_duration;
        bool tick;

        static FixedUpdateTick null_tick();
        bool is_null() const;
        operator bool() const;
    };

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