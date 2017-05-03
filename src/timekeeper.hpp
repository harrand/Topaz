#ifndef TIMEKEEPER_HPP
#define TIMEKEEPER_HPP
#include <chrono>
#include <vector>
#include <thread>
#include <functional>

class TimeKeeper
{
public:
	TimeKeeper();
	TimeKeeper(const TimeKeeper& copy) = default;
	TimeKeeper(TimeKeeper&& move) = default;
	TimeKeeper& operator=(const TimeKeeper& rhs) = default;
	
	void update();
	void reload();
	float getRange() const;
	bool millisPassed(float millis) const;
private:
	unsigned long before, after;
	time_t previous, now;
};

class TimeProfiler
{
public:
	TimeProfiler();
	TimeProfiler(const TimeProfiler& copy) = default;
	TimeProfiler(TimeProfiler&& move) = default;
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

class Scheduler
{
public:
	template<class ReturnType, class... Args>
	static inline void syncDelayedTask(unsigned int millisDelay, std::function<ReturnType(Args...)> f, Args... args)
	{
		std::this_thread::sleep_for(std::chrono::duration<unsigned int, std::milli>(millisDelay));
		f(args...);
	}

	template<class ReturnType, class... Args>
	static inline void asyncDelayedTask(unsigned int millisDelay, std::function<ReturnType(Args...)> f, Args... args)
	{
		std::thread(syncDelayedTask<ReturnType, Args...>, millisDelay, f, args...).detach();
	}
	/*
	template<class ReturnType>
	static inline void syncDelayedTask(unsigned int millisDelay, std::function<ReturnType> f)
	{
		std::this_thread::sleep_for(std::chrono::duration<unsigned int, std::milli>(millisDelay));
		f();
	}

	template<class ReturnType>
	static inline void asyncDelayedTask(unsigned int millisDelay, std::function<ReturnType> f)
	{
		std::thread(Scheduler::syncDelayedTask<ReturnType>, millisDelay, f).detach();
	}
	*/
};

#endif // TIMEKEEPER_HPP