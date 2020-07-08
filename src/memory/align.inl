#include <algorithm>
#include <limits>
#include "core/debug/assert.hpp"

namespace tz::mem
{
	template<typename T, std::size_t Align>
	bool is_aligned(const T& t)
	{
		// Bow before the dark god of pointer-integer conversion
		auto t_addr = reinterpret_cast<std::uintptr_t>(&t);
		return (t_addr & (Align - 1)) == 0;
	}
}