#include "tz/core/messaging.hpp"
#include "tz/core/debug.hpp"

// scenario 1: message to just assign an int.
struct assignment_msg
{
	int new_val;
};
static_assert(tz::message<assignment_msg>);

class deferred_assignment : public tz::message_receiver<assignment_msg>
{
public:
	int x = 0;
	void update()
	{
		tz::message_receiver<assignment_msg>::process_messages();
	}
private:
	virtual void process_message(const assignment_msg& msg) override final
	{
		this->x = msg.new_val;
	}
};

void scenario1()
{
	deferred_assignment sys;
	tz::assert(sys.x == 0);
	sys.send_message({.new_val = 5});
	// message is not instantly processed until update() is called.
	tz::assert(sys.x == 0);
	sys.update();
	tz::assert(sys.x == 5);
}

// scenario 2: assign an int, but based on a timestamp.
// the most recent message (based on timestamp) is carried out during processing, the rest are dropped.

struct timestamped_assignment_msg
{
	int new_val;
	float timestamp;
};

class deferred_timestamped_assignment : public tz::message_receiver<timestamped_assignment_msg>
{
public:
	int x = 0;
	void update()
	{
		tz::message_receiver<timestamped_assignment_msg>::process_messages();
		this->most_recent_timestamp = -1.0f;
	}
private:
	float most_recent_timestamp = -1.0f;
	virtual bool on_send_message(const timestamped_assignment_msg& msg) override final
	{
		// return true if this is the newest timestamp we've seen.
		if(msg.timestamp > this->most_recent_timestamp)
		{
			this->most_recent_timestamp = msg.timestamp;
			return true;
		}
		return false;
	}
	virtual void process_message(const timestamped_assignment_msg& msg) override final
	{
		this->x = msg.new_val;
	}
};

void scenario2()
{
	// this receiver drops new messages if their timestamp was earlier than whatever its most recent timestamp was.
	deferred_timestamped_assignment sys;
	tz::assert(sys.x == 0);
	// no dropping of messages here. 1st is newest, but then 2nd is newest.
	sys.send_message({.new_val = 5, .timestamp = 1.0f});
	sys.send_message({.new_val = 7, .timestamp = 2.0f});
	sys.update();
	// most recent was 7. should be 7.
	tz::assert(sys.x == 7);

	sys.x = 0;
	// note: these timestamps are smaller than before
	// however these both wont be dropped - update() also resets the max timestamp.
	sys.send_message({.new_val = 95, .timestamp = 0.5f});
	sys.send_message({.new_val = 3, .timestamp = 0.4f});
	tz::assert(sys.x == 0);
	sys.update();
	// most recent was 95, as timestamp was higher (remember, this is not chronological). 3 assignment should've been dropped.
	tz::assert(sys.x == 95);
}

int main()
{
	scenario1();
	scenario2();
}
