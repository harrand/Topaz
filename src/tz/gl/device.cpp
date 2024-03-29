#include "tz/gl/device.hpp"
#include "tz/core/profile.hpp"
#include "tz/lua/api.hpp"
#include <memory>

namespace tz::gl
{
	std::unique_ptr<device> dev = nullptr;

	device& get_device()
	{
		if(dev == nullptr)
		{
			TZ_PROFZONE("get_device() - first-time creation", 0xFFAAAA00);
			dev = std::make_unique<device>();	
		}
		return *dev;
	}

	void destroy_device()
	{
		if(dev != nullptr)
		{
			TZ_PROFZONE("tz::gl - destroy device", 0xFFAAAA00);
			// Why the hell this magic instead of assigning to nullptr, you ask?
			// It's very possible ~device() ends up invoking tz::gl::get_device(), which will be null during this dtor, so it tries to reconstruct it again and cause real problems. So the steps to fix this are:
			// Invoke dtor, tz::gl::get_device() remains valid throughout dtor usage this time.
			(*dev).~device();
			// Release the raw ptr from the unique_ptr. `dev` is now nullptr.
			auto* p = dev.release();
			// Free the memory without calling the dtor a second time.
			operator delete(p);
		}
	}

}
