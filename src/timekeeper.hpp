#ifndef TIMEKEEPER_HPP
#define TIMEKEEPER_HPP
#include <chrono>

class TimeKeeper
{
public:
	TimeKeeper();
	void update();
	void reload();
	bool millisPassed(float millis);
	float getRange();
private:
	unsigned long before, after;
	time_t previous, now;
};

#endif // TIMEKEEPER_HPP