#ifndef TIMEKEEPER_HPP
#define TIMEKEEPER_HPP
#include <chrono>

class TimeKeeper
{
public:
	TimeKeeper();
	void update();
	void reload();
	float getRange() const;
	bool millisPassed(float millis) const;
private:
	unsigned long before, after;
	time_t previous, now;
};

#endif // TIMEKEEPER_HPP