#ifndef TIME_HPP
#define TIME_HPP
#include <vector>
#include <thread>
#include <functional>
#include <ctime>

/*
	Use this to schedule, record time or pretty much do anything that requires timing.
*/
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
	float get_range() const;
	bool millis_passed(float millis) const;
private:
	long long int before, after;
};

/*
	Specialised Timer that can be used to calculate FPS during runtime.
*/
class TimeProfiler
{
public:
	TimeProfiler();
	TimeProfiler(const TimeProfiler& copy) = default;
	TimeProfiler(TimeProfiler&& move) = default;
	~TimeProfiler() = default;
	TimeProfiler& operator=(const TimeProfiler& rhs) = default;
	
	void begin_frame();
	void end_frame();
	void reset();
	float get_delta_average() const;
	float get_last_delta() const;
	unsigned int get_fps() const;
private:
	std::vector<float> deltas;
	Timer tk;
};

namespace tz::time
{
	inline long long int now()
	{
		return std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
	}
}
#endif