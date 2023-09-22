#ifndef TOPAZ_CORE_ENDIAN_HPP
#define TOPAZ_CORE_ENDIAN_HPP

namespace tz
{
	enum class endian
	{
		little,
		big
	};

	endian get_local_machine_endianness();
}

#endif // TOPAZ_CORE_ENDIAN_HPP