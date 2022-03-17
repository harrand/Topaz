#include "core/allocators/linear.hpp"
#include "core/assert.hpp"
#include <type_traits>
#include <vector>
#include <numeric>

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
	std::span<const int> storage_view{reinterpret_cast<int*>(i5.data.data()), 5};

	std::vector<int, tz::LinearAllocator<int>> ints(make_allocator<int, 5>(i5));
	tz_assert(ints.get_allocator().get_offset() == 0, "Empty vector on allocator has already allocated memory?");
	// Alloc 1 int
	ints.resize(1, 0);
	tz_assert(ints.get_allocator().get_offset() == sizeof(int), "Allocator is giving off the wrong offset after a single allocation of one object. Expected %zu, got %zu", sizeof(int), ints.get_allocator().get_offset());
	// Alloc 2 more ints (the 0 from befoer will be copied over into the new data store).
	ints.push_back(1);
	tz_assert(ints.get_allocator().get_offset() == 3 * sizeof(int), "Allocator is giving off the wrong offset after a single allocation of one object. Expected %zu, got %zu", 3 * sizeof(int), ints.get_allocator().get_offset());

	// Storage should look like {0, 0, 1}
	tz_assert(storage_view[0] == 0, "Unexpected value at storage view element %zu. Expected value %d, got %d", 0, 0, storage_view[0]);
	tz_assert(storage_view[1] == 0, "Unexpected value at storage view element %zu. Expected value %d, got %d", 1, 0, storage_view[1]);
	tz_assert(storage_view[2] == 1, "Unexpected value at storage view element %zu. Expected value %d, got %d", 2, 1, storage_view[2]);

	tz::ScratchLinearAllocator<int, 10> s10;
	std::vector<int, tz::ScratchLinearAllocator<int, 10>> ints2(s10);
	ints2.resize(10);
}

int main()
{
	linear_allocator_tests();
}
