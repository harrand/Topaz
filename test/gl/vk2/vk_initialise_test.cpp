#include "core/assert.hpp"
#include "core/tz.hpp"
#include "gl/impl/backend/vk2/tz_vulkan.hpp"

int main()
{
	tz::initialise
	({
		.name = "vk_initialise_test",
		.flags = {tz::ApplicationFlag::HiddenWindow}
	});
	tz::terminate();
	return 0;
}
