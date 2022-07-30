#include "tz/core/allocators/adapter.hpp"
#include "tz/core/allocators/linear.hpp"
#include "tz/core/allocators/malloc.hpp"
#include "tz/core/allocators/null.hpp"
#include "tz/core/allocators/stack.hpp"
#include "tz/core/assert.hpp"
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

void linear_allocator_tests()
{
	// First create some storage we can use.
	tz::Mallocator mallocator;
	tz::Blk scratch = mallocator.allocate(256);
	{
		tz::LinearAllocator lalloc{scratch};
		tz::Blk blk = lalloc.allocate(256);
		tz_assert(lalloc.owns(blk), "LinearAllocator allocated its entire contents, but says it doesn't own the resultant block");
		lalloc.deallocate(blk);
		tz_assert((blk = lalloc.allocate(1)) != tz::nullblk, "LinearAllocator thinks it ran out of space but I deallocated it to empty, the next allocation returned the null block.");
		lalloc.deallocate(blk);
		tz_assert(lalloc.allocate(257) == tz::nullblk, "LinearAllocator ran out of space, but didn't output the null block.");

		// The adapter linear allocator will happily overwrite any previous allocations.
		tz::AllocatorAdapter<int, tz::LinearAllocator> int_lalloc{lalloc};
		std::vector<int, tz::AllocatorAdapter<int, tz::LinearAllocator>> ints{int_lalloc};
		ints.reserve(4);
		ints.push_back(1);
		ints.push_back(2);

		// Check memory values
		tz_assert(*(reinterpret_cast<int*>(scratch.ptr) + 0) == 1, "LinearAllocator is allocating in a dodgy area in its arena.");
		tz_assert(*(reinterpret_cast<int*>(scratch.ptr) + 1) == 2, "LinearAllocator is allocating in a dodgy area in its arena.");

		ints.clear();
		ints.push_back(3);
		tz_assert(*(reinterpret_cast<int*>(scratch.ptr) + 0) == 3, "LinearAllocator is allocating in a dodgy area in its arena (post-dealloc)");
	}
	mallocator.deallocate(scratch);
}

int main()
{
	null_allocator_tests();
	mallocator_test();
	stack_allocator_tests();
	linear_allocator_tests();
}
