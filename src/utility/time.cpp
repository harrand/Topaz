#include "time.hpp"
#include <cmath>

Timer::Timer()
{
	this->reload();
}

void Timer::update()
{
	// Assign 'after' to the current number of millis past since Epoch.
	this->after = tz::utility::time::now();
}

void Timer::reload()
{
	// Assign both to the number of millis past since Epoch, so that the range is zero.
	this->before = tz::utility::time::now();
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
		return 0.0f;
	return this->deltas.back();
}

unsigned int TimeProfiler::get_fps() const
{
	return static_cast<unsigned int>(1000/this->get_delta_average());
}

FrameScheduler::FrameScheduler(unsigned int number_of_frames, unsigned int fps, bool loop): time(0.0f), number_of_frames(number_of_frames), fps(fps), loop(loop){}

void FrameScheduler::to_beginning()
{
	this->time = 0.0f;
}

void FrameScheduler::to_end()
{
	this->time = this->get_end_time();
}

void FrameScheduler::to_frame(unsigned int frame)
{
	unsigned int expected_frame = std::clamp<unsigned int>(frame, 0, this->number_of_frames);
	this->time = static_cast<float>(expected_frame) / this->fps;
}

void FrameScheduler::update(float delta_millis)
{
	// this->time is in seconds, so divide by 1000.
	this->time += delta_millis / 1000.0f;
    if(this->time > this->get_end_time())
        this->time -= this->get_end_time();
}

void FrameScheduler::set_number_of_frames(unsigned int number_of_frames)
{
	this->number_of_frames = number_of_frames;
}

unsigned int FrameScheduler::get_number_of_frames() const
{
	return this->number_of_frames;
}

unsigned int FrameScheduler::get_current_frame() const
{
	return std::clamp<unsigned int>(static_cast<unsigned int>(std::floor(this->time * this->fps)), 0, this->get_number_of_frames() - 1);
}

bool FrameScheduler::finished() const
{
	return this->time >= (this->number_of_frames / this->fps);
}

float FrameScheduler::get_end_time() const
{
    return static_cast<float>(this->number_of_frames) / this->fps;
}

namespace tz::utility::time
{
    long long int now()
    {
        return std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
    }
}