#include "tz/topaz.hpp"
#include "tz/gpu/device.hpp"

int main()
{
	tz::initialise();

	tz::gpu::hardware gpu = tz::gpu::find_best_hardware();
	tz::gpu::device_handle dev = tz::gpu::create_device(gpu);
	tz::gpu::destroy_device(dev);

	tz::terminate();

	return 0;
}
