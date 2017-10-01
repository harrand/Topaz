#ifndef TIMEKEEPER_HPP
#define TIMEKEEPER_HPP
#include <vector>
#include <thread>
#include <functional>
#include <ctime>

class TimeKeeper
{
public:
	TimeKeeper();
	TimeKeeper(const TimeKeeper& copy) = default;
	TimeKeeper(TimeKeeper&& move) = default;
	~TimeKeeper() = default;
	TimeKeeper& operator=(const TimeKeeper& rhs) = default;
	
	void update();
	void reload();
	float getRange() const;
	bool millisPassed(float millis) const;
private:
	std::chrono::milliseconds before, after;
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
	float getDeltaAverage();
	float getLastDelta() const;
	unsigned int getFPS();
private:
	std::vector<float> deltas;
	TimeKeeper tk;
};

namespace tz
{
	namespace scheduler
	{
		template<class ReturnType, class... Args>
		static inline void syncDelayedTask(unsigned int milliseconds_delay, std::function<ReturnType(Args...)> f, Args... args)
		{
			std::this_thread::sleep_for(std::chrono::duration<unsigned int, std::milli>(milliseconds_delay));
			f(args...);
		}
		template<class ReturnType, class... Args>
		static inline void asyncDelayedTask(unsigned int milliseconds_delay, std::function<ReturnType(Args...)> f, Args... args)
		{
			std::thread(syncDelayedTask<ReturnType, Args...>, milliseconds_delay, f, args...).detach();
		}
	}
}
#endif // TIMEKEEPER_HPP