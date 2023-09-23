#include "tz/core/endian.hpp"
#include "tz/core/debug.hpp"

namespace tz
{
	endian get_local_machine_endianness()
	{
		unsigned char endian_test[2] = {1, 0};
		short x;
		x = *reinterpret_cast<short*>(endian_test);
		switch(x)
		{
			case 1:
				return endian::little;
			break;
			case 256:
				return endian::big;
			break;
			default:
				tz::error("Unrecognised endianness! endian_test interpreted as short = %d", x);
				return endian::little;
			break;
		}
	}

	bool is_big_endian()
	{
		return get_local_machine_endianness() == endian::big;
	}

	bool is_little_endian()
	{
		return get_local_machine_endianness() == endian::little;
	}
}