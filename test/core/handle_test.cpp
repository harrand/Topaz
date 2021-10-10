#include "core/handle.hpp"
#include "core/assert.hpp"

struct HandleGenerator
{
public:
	HandleGenerator()
	{
		inst_count++;
	}
	HandleGenerator(const HandleGenerator& copy):
	HandleGenerator(){}
	
	HandleGenerator(HandleGenerator&& move):
	HandleGenerator(){}

	~HandleGenerator()
	{
		inst_count--;
	}
	static inline unsigned int inst_count = 0;
};

int main()
{
	tz_assert(HandleGenerator::inst_count == 0, "HandleGenerator instance count started at %zu, not zero", HandleGenerator::inst_count);
	HandleGenerator g;	
	tz_assert(HandleGenerator::inst_count == 1, "HandleGenerator instance count started at %zu, not zero", HandleGenerator::inst_count);
	return 0;
}
