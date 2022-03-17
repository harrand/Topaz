#include "core/allocators/adapter.hpp"
#include "core/allocators/malloc.hpp"
#include "core/allocators/null.hpp"
#include "core/allocators/linear.hpp"
#include "core/allocators/stack.hpp"
#include "core/assert.hpp"
#include <type_traits>
#include <vector>
#include <deque>
#include <numeric>

void null_allocator_tests()
{
	tz::NullAllocator na;
	tz::Blk blk = na.allocate(1);
	tz_assert(blk == tz::nullblk, "NullAllocator did not return a null allocation.");
	tz_assert(na.owns(blk), "NullAllocator thinks it doesn't own the block it created.");
	na.deallocate(blk);
}

void mallocator_test()
{
	tz::Mallocator ma;
	tz::Blk blk = ma.allocate(1024);
	tz_assert(blk != tz::nullblk, "Mallocator failed to alloc 1024 bytes.");
	tz_assert(ma.owns(blk), "Mallocator thinks it does not own the block it created.");

	std::vector<int, tz::AllocatorAdapter<int, tz::Mallocator>> ints{1, 2, 3, 4};
	ints.clear();
	ints.push_back(5);

	std::deque<int, tz::AllocatorAdapter<int, tz::Mallocator>> int_deque;
	int_deque.push_back(0);
	int_deque.push_front(1);
	int_deque.clear();
	int_deque.resize(10);
	std::iota(int_deque.begin(), int_deque.end(), 0);
}

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

void stack_allocator_tests()
{
	tz::StackAllocator<64> s;
	auto blk2 = s.allocate(1);
	tz_assert(s.owns(blk2) && !s.owns(tz::nullblk), "StackAllocator says it doesn't own a block it allocated, or thinks it owns nullblk");
	s.deallocate(blk2);

	using LocalAllocator = tz::StackAllocator<64>;
	std::vector<int, tz::AllocatorAdapter<int, LocalAllocator>> ints2;
	ints2.resize(4);
	ints2.push_back(5);
	ints2.push_back(6);
}

int main()
{
	null_allocator_tests();
	linear_allocator_tests();
	mallocator_test();
	stack_allocator_tests();
}
