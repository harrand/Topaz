#include "core/handle.hpp"
#include "core/assert.hpp"
#include <vector>

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

	tz::Handle<unsigned int> get() const
	{
		return {tz::HandleValue{inst_count}};
	}

	~HandleGenerator()
	{
		inst_count--;
	}
	static inline unsigned int inst_count = 0;
};

int main()
{
	auto get = [](const auto& handle)
	{
		return static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
	};

	std::vector<HandleGenerator> gens;
	auto h1 = gens.emplace_back().get();
	tz_assert(h1 != tz::nullhand, "Valid handle == nullhand. Impossibru");
	auto h2 = gens.emplace_back().get();
	tz_assert(h2 != tz::nullhand && h2 != h1, "Valid handle == nullhand. Impossibru");
	gens.clear();
	auto h3 = gens.emplace_back().get();
	tz_assert(h3 == h1, "Handles do not make sense");
	return 0;
}
