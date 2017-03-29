#include "timekeeper.hpp"

TimeKeeper::TimeKeeper()
{
	this->reload();
}

void TimeKeeper::update()
{
	// Assign 'after' to the current number of millis past since Epoch.
	this->after = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
}

void TimeKeeper::reload()
{
	// Assign both to the number of millis past since Epoch, so that the range is zero.
	this->before = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
	this->after = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
}

float TimeKeeper::getRange() const
{
	return (this->after) - (this->before);
}

bool TimeKeeper::millisPassed(float millis) const
{
	return (this->getRange() > millis);
}

TimeProfiler::TimeProfiler(): tk(TimeKeeper()){}

void TimeProfiler::beginFrame()
{
	this->tk.update();
}

void TimeProfiler::endFrame()
{
	this->deltas.push_back(this->tk.getRange());
	this->tk.reload();
}

void TimeProfiler::reset()
{
	this->deltas.clear();
	this->tk.reload();
}

float TimeProfiler::getDeltaAverage() const
{
	float total = 0.0f;
	for(unsigned int i = 0; i < this->deltas.size(); i++)
		total += this->deltas.at(i);
	if(total == 0.0f || this->deltas.size() == 0)
		return 1000;
	return total / this->deltas.size();
}

unsigned int TimeProfiler::getFPS() const
{
	return (int)(1000/this->getDeltaAverage());
}