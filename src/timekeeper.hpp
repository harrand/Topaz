#ifndef TIMEKEEPER_HPP
#define TIMEKEEPER_HPP
#include <chrono>
#include <vector>

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

class TimeProfiler
{
public:
	TimeProfiler();
	void beginFrame();
	void endFrame();
	void reset();
	float getDeltaAverage() const;
	unsigned int getFPS() const;
private:
	std::vector<float> deltas;
	TimeKeeper tk;
};

#endif // TIMEKEEPER_HPP