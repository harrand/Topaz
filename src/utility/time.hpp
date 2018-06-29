#ifndef TIME_HPP
#define TIME_HPP
#include <vector>
#include <functional>

/**
* Use this to schedule, record time or pretty much do anything that requires timing.
*/
class Timer
{
public:
	/// Construct and synchronise the Timer.
	Timer();

	/**
	 * Invoke every frame, so that Timer::get_range and Timer::millis_passed return accurate values.
	 */
	void update();
	/**
	 * Invoke whenever the Timer::get_range should return 0.0f.
	 */
	void reload();
	/**
	 * Returns the duration since the last reload.
	 * @return - Time taken between the last invocation of Timer::update and Timer::reload, in milliseconds
	 */
	float get_range() const;
	/**
	 * Query whether a specified duration has passed since the last reload.
	 * @param millis - Number of milliseconds in the specified duration
	 * @return - True if the duration has passed. False otherwise
	 */
	bool millis_passed(float millis) const;
private:
	/// Times in milliseconds.
	long long int before, after;
};

/**
* Specialised Timer that can be used to calculate FPS during runtime.
*/
class TimeProfiler
{
public:
	/// Construct the underlying Timer.
	TimeProfiler();

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
	 * Get the average frame-delta.
	 * @return - Average time taken between each frame, in milliseconds.
	 */
	float get_delta_average() const;
	/**
	 * Get the time taken for the last frame, in milliseconds.
	 * @return - Time taken since the last frame, in milliseconds.
	 */
	float get_last_delta() const;
	/**
	 * Get the average FPS.
	 * @return - Average number of frames processed per second
	 */
	unsigned int get_fps() const;
private:
	/// Container of all the time-deltas, in milliseconds.
	std::vector<float> deltas;
	/// Underlying Timer object.
	Timer tk;
};

namespace tz::utility::time
{
	/**
	 * Returns the current local time, in milliseconds.
	 * @return - Current local-time
	 */
	long long int now();

    namespace scheduler
    {
        /**
         * Invokes a function synchronously with specified arguments after a specified delay.
         * @tparam ReturnType - Return type of the function
         * @tparam Args - Argument types of the function
         * @param milliseconds_delay - Number of milliseconds to elapse before executing the function.
         * @param f - The function to execute
         * @param args - Arguments to emplace into the function invocation
         */
        template<class ReturnType, class... Args>
        void sync_delayed_function(unsigned int milliseconds_delay, std::function<ReturnType(Args...)> f, Args... args);
        /**
         * Invokes a function asynchronously with specified arguments after a specified delay.
         * @tparam ReturnType - Return type of the function
         * @tparam Args - Argument types of the function
         * @param milliseconds_delay - Number of milliseconds to elapse before executing the function.
         * @param f - The function to execute
         * @param args - Arguments to emplace into the function invocation
         */
        template<class ReturnType, class... Args>
        void async_delayed_function(unsigned int milliseconds_delay, std::function<ReturnType(Args...)> f, Args... args);
    }
}

#include "time.inl"

#endif