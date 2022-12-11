#include "tz/gl/device.hpp"
#include <memory>

namespace tz::gl
{
	std::unique_ptr<Device> dev = nullptr;

	Device& device()
	{
		if(dev == nullptr)
		{
			dev = std::make_unique<Device>();	
		}
		return *dev;
	}

	void destroy_device()
	{
		if(dev != nullptr)
		{
			// Why the hell this magic instead of assigning to nullptr, you ask?
			// It's very possible ~Device() ends up invoking tz::gl::device(), which will be null during this dtor, so it tries to reconstruct it again and cause real problems. So the steps to fix this are:
			// Invoke dtor, tz::gl::device() remains valid throughout dtor usage this time.
			(*dev).~Device();
			// Release the raw ptr from the unique_ptr. `dev` is now nullptr.
			auto* p = dev.release();
			// Free the memory without calling the dtor a second time.
			operator delete(p);
		}
	}
}
