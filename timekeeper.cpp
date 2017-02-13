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

float TimeKeeper::getRange()
{
	return (this->after) - (this->before);
}

bool TimeKeeper::millisPassed(float millis)
{
	return (this->getRange() > millis);
}