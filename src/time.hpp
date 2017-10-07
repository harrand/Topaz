#ifndef TIME_HPP
#define TIME_HPP
#include <vector>
#include <thread>
#include <functional>
#include <ctime>

class Timer
{
public:
	Timer();
	Timer(const Timer& copy) = default;
	Timer(Timer&& move) = default;
	~Timer() = default;
	Timer& operator=(const Timer& rhs) = default;
	
	void update();
	void reload();
	float getRange() const;
	bool millisPassed(float millis) const;
private:
	long long int before, after;
};

class TimeProfiler
{
public:
	TimeProfiler();
	TimeProfiler(const TimeProfiler& copy) = default;
	TimeProfiler(TimeProfiler&& move) = default;
	~TimeProfiler() = default;
	TimeProfiler& operator=(const TimeProfiler& rhs) = default;
	
	void beginFrame();
	void endFrame();
	void reset();
	float getDeltaAverage() const;
	float getLastDelta() const;
	unsigned int getFPS() const;
private:
	std::vector<float> deltas;
	Timer tk;
};

namespace tz::time
{
	namespace scheduler
	{
		template<class ReturnType, class... Args>
		inline void syncDelayedTask(unsigned int milliseconds_delay, std::function<ReturnType(Args...)> f, Args... args)
		{
			std::this_thread::sleep_for(std::chrono::duration<unsigned int, std::milli>(milliseconds_delay));
			f(args...);
		}
		template<class ReturnType, class... Args>
		inline void asyncDelayedTask(unsigned int milliseconds_delay, std::function<ReturnType(Args...)> f, Args... args)
		{
			std::thread(syncDelayedTask<ReturnType, Args...>, milliseconds_delay, f, args...).detach();
		}
	}
}
#endif // TIMEKEEPER_HPP