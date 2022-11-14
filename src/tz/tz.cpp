#include "tz/tz.hpp"
#include "hdk/hdk.hpp"

#include "impl/vulkan/tzvk.hpp"

namespace tz
{
	#if TOPAZ_VULKAN
		namespace impl = impl_vk;
	#else
		static_assert(false, "Internal: Unrecognised Graphics API");
	#endif

	void initialise()
	{
		hdk::initialise();
		impl::initialise();
	}

	void terminate()
	{
		hdk::terminate();
		impl::terminate();
	}
}
