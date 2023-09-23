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
	bool is_big_endian();
	bool is_little_endian();
}

#endif // TOPAZ_CORE_ENDIAN_HPP