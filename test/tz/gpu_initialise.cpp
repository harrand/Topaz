#include "tz/topaz.hpp"
#include "tz/gpu/device.hpp"
#include <vector>

int main()
{
	tz::initialise();

	std::size_t hardware_count;
	tz::gpu::error_code res;
	res = tz::gpu::iterate_hardware({}, &hardware_count);
	std::vector<tz::gpu::hardware> hardware(hardware_count);
	res = tz::gpu::iterate_hardware(hardware);

	tz::terminate();

	return 0;
}
