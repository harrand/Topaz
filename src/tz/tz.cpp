#include "tz/tz.hpp"
#include "hdk/hdk.hpp"
#include "hdk/data/version.hpp"
#include "hdk/debug.hpp"

#include "impl/vulkan/tzvk.hpp"

namespace tz
{
	#if TOPAZ_VULKAN
		namespace impl = impl_vk;
	#else
		static_assert(false, "Internal: Unrecognised Graphics API");
	#endif

//------------------------------------------------------------------------------------------------

	void initialise()
	{
		hdk::initialise();
		impl::initialise();

		hdk::report("Topaz %s - Initialised", get_version().to_string().c_str());
	}

//------------------------------------------------------------------------------------------------

	void terminate()
	{
		hdk::terminate();
		impl::terminate();
		hdk::report("Topaz - Terminated");
	}

//------------------------------------------------------------------------------------------------

	hdk::version get_version()
	{
		return hdk::version::from_binary_string(TZ_VER);
	}
}
