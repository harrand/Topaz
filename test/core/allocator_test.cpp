#include "core/allocators/linear.hpp"
#include "core/assert.hpp"
#include <type_traits>
#include <vector>

template<typename T, std::size_t N>
struct Storage
{
	alignas(alignof(T)) std::array<char, sizeof(T) * N> data;
};

template<typename T, std::size_t N>
tz::LinearAllocator<T> make_allocator(Storage<T, N>& storage)
{
	return {{reinterpret_cast<std::byte*>(storage.data.data()), sizeof(T) * N}};
}

void linear_allocator_tests()
{
	Storage<int, 5> i5;
	auto l5 = make_allocator<int, 5>(i5);

	std::vector<int, tz::LinearAllocator<int>> ints(l5);
	ints.resize(5);

	tz::ScratchLinearAllocator<int, 10> s10;
	std::vector<int, tz::ScratchLinearAllocator<int, 10>> ints2(s10);
	ints2.resize(10);
}

int main()
{
	linear_allocator_tests();
}
