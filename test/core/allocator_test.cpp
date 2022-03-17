#include "core/allocators/adapter.hpp"
#include "core/allocators/malloc.hpp"
#include "core/allocators/null.hpp"
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
	mallocator_test();
	stack_allocator_tests();
}
