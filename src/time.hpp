#ifndef TIME_HPP
#define TIME_HPP
#include <vector>
#include <thread>
#include <functional>
#include <ctime>

/**
* Use this to schedule, record time or pretty much do anything that requires timing.
*/
class Timer
{
public:
	Timer();
	Timer(const Timer& copy) = default;
	Timer(Timer&& move) = default;
	~Timer() = default;
	Timer& operator=(const Timer& rhs) = default;

	/**
	 * Invoke every frame, so that Timer::get_range and Timer::millis_passed return accurate values.
	 */
	void update();
	/*
	 * Invoke whenever the Timer::get_range should return 0.0f.
	 */
	void reload();
	/**
	 * Returns the time taken between the last invocation of Timer::update and Timer::reload.
	 */
	float get_range() const;
	/**
	 * Returns whether Timer::get_range returns a time >= the @param millis.
	 */
	bool millis_passed(float millis) const;
private:
	long long int before, after;
};

/**
* Specialised Timer that can be used to calculate FPS during runtime.
*/
class TimeProfiler
{
public:
	TimeProfiler();
	TimeProfiler(const TimeProfiler& copy) = default;
	TimeProfiler(TimeProfiler&& move) = default;
	~TimeProfiler() = default;
	TimeProfiler& operator=(const TimeProfiler& rhs) = default;

	/**
	 * Invoke this at the beginning of your frame construction in the game-loop.
	 */
	void begin_frame();
	/**
	 * Invoke this at the end of your frame construction in the game-loop.
	 */
	void end_frame();
	/**
	 * Purges all existing time-deltas from the delta-vector. Invoke this to reset the fps-counter.
	 */
	void reset();
	/**
	 * Returns the average time taken between each frame.
	 */
	float get_delta_average() const;
	/**
	 * Returns the time taken for the most recent frame.
	 */
	float get_last_delta() const;
	/**
	 * Returns the average number of frames processed per second.
	 */
	unsigned int get_fps() const;
private:
	std::vector<float> deltas;
	Timer tk;
};

namespace tz::time
{
	/**
	* Returns the current local time.
	*/
	inline long long int now()
	{
		return std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
	}
}
#endif