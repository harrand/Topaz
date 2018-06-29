#include <thread>
#include <chrono>

namespace tz::utility::time::scheduler
{
    template<class ReturnType, class... Args>
    inline void sync_delayed_function(unsigned int milliseconds_delay, std::function<ReturnType(Args...)> f, Args... args)
    {
        std::this_thread::sleep_for(std::chrono::duration<unsigned int, std::milli>(milliseconds_delay));
        f(args...);
    }

    template<class ReturnType, class... Args>
    inline void async_delayed_task(unsigned int milliseconds_delay, std::function<ReturnType(Args...)> f, Args... args)
    {
        std::thread(sync_delayed_function<ReturnType, Args...>, milliseconds_delay, f, args...).detach();
    }
}