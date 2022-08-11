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
			dev = nullptr;
		}
	}
}
