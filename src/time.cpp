#include "time.hpp"
#include <chrono>

Timer::Timer()
{
	this->reload();
}

void Timer::update()
{
	// Assign 'after' to the current number of millis past since Epoch.
	this->after = tz::time::now();
}

void Timer::reload()
{
	// Assign both to the number of millis past since Epoch, so that the range is zero.
	this->before = tz::time::now();
	this->after = after;
}

float Timer::get_range() const
{
	return (this->after) - (this->before);
}

bool Timer::millis_passed(float millis) const
{
	return (this->get_range() > millis);
}

TimeProfiler::TimeProfiler(): tk(Timer()){}

void TimeProfiler::begin_frame()
{
	this->tk.update();
}

void TimeProfiler::end_frame()
{
	this->deltas.push_back(this->tk.get_range());
	this->tk.reload();
}

void TimeProfiler::reset()
{
	this->deltas.clear();
	this->tk.reload();
}

float TimeProfiler::get_delta_average() const
{
	float total = 0.0f;
	for(float delta : this->deltas)
		total += delta;
	if(deltas.size() == 0)
		return 5.0f/3.0f;
	return total / deltas.size();
}

float TimeProfiler::get_last_delta() const
{
	// this is useful for updating physics engine to prevent updates from changing so strongly every second
	if(this->deltas.empty())
		return 5.0f/3.0f;
	return this->deltas.back();
}

unsigned int TimeProfiler::get_fps() const
{
	return static_cast<unsigned int>(1000/this->get_delta_average());
}