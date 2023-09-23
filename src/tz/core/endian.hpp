#ifndef TOPAZ_CORE_ENDIAN_HPP
#define TOPAZ_CORE_ENDIAN_HPP
#include <concepts>

namespace tz
{
	/**
	 * @ingroup tz_core
	 * Indicates the endianness of scalar types.
	 **/
	enum class endian
	{
		/// Little Endian
		little,
		/// Big Endian
		big
	};

	/**
	 * @ingroup tz_core
	 * Query the endianness of the running machine.
	 * @return `endian::little` if machine is little-endian, or `endian::big` otherwise.
	 **/
	endian get_local_machine_endianness();
	/**
	 * @ingroup tz_core
	 * Query as to whether the current system is big-endian.
	 * @note Equivalent to `tz::get_local_machine_endianness() == endian::big`
	 **/
	bool is_big_endian();
	/**
	 * @ingroup tz_core
	 * Query as to whether the current system is littel-endian.
	 * @note Equivalent to `tz::get_local_machine_endianness() == endian::little`
	 **/
	bool is_little_endian();

	/**
	 * @ingroup tz_core
	 * Given an integral type, make a copy of the value, swap its bytes, and then return it.
	 * If the value for example is little-endian, then the result will be its big-endian variant.
	 * @param value Value to byte swap.
	 * @return Copy of value, with endianness swapped.
	 **/
	template<std::integral T>
	T endian_byte_swap(T value)
	{
		char* bytes = reinterpret_cast<char*>(&value);
		std::size_t sz = sizeof(T);
		for(std::size_t i = 0; i < sz / 2; i++)
		{
			std::swap(bytes[i], bytes[sz - i - 1]);
		}
		return value;
	}

	/**
	 * @ingroup tz_core
	 * Retrieve a scalar value in big-endian form. If the system is detected to be big-endian, the value is unchanged.
	 * @param value The value to ensure is big-endian.
	 * @return Byte-swap of the value if the system is little-endian, otherwise the same value.
	 **/
	template<std::integral T>
	T big_endian(T value)
	{
		if(is_big_endian())
		{
			return value;
		}
		return endian_byte_swap(value);
	}

	/**
	 * @ingroup tz_core
	 * Retrieve a scalar value in little-endian form. If the system is detected to be little-endian, the value is unchanged.
	 * @param value The value to ensure is little-endian.
	 * @return Byte-swap of the value if the system is big-endian, otherwise the same value.
	 **/
	template<std::integral T>
	T little_endian(T value)
	{
		if(is_little_endian())
		{
			return value;
		}
		return endian_byte_swap(value);
	}
}

#endif // TOPAZ_CORE_ENDIAN_HPP