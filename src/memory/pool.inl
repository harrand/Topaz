#include <algorithm>

namespace tz::mem
{
	namespace
	{
		std::size_t byte_distance(void* a, void* b)
		{
			std::ptrdiff_t signed_distance = std::distance(a, b);
			return static_cast<std::size_t>(std::abs(signed_distance));
		}
	}
	
	template<typename T>
	UniformPool<T>::UniformPool(void* begin, void* end): UniformPool<T>(begin, byte_distance(begin, end)){}
	
	template<typename T>
	UniformPool<T>::UniformPool(void* begin, std::size_t size_bytes): begin(begin), size_bytes(size_bytes){}
	
	
}