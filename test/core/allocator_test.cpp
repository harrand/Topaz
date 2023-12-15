#include "tz/core/memory/allocators/adapter.hpp"
#include "tz/core/memory/allocators/linear.hpp"
#include "tz/core/memory/allocators/malloc.hpp"
#include "tz/core/memory/allocators/null.hpp"
#include "tz/core/memory/allocators/stack.hpp"
#include "tz/core/memory/allocators/slab.hpp"
#include "tz/core/debug.hpp"
#include <type_traits>
#include <vector>
#include <deque>
#include <numeric>

void null_allocator_tests()
{
	tz::null_allocator na;
	tz::memblk blk = na.allocate(1);
	tz::assert(blk == tz::nullblk, "null_allocator did not return a null allocation.");
	tz::assert(na.owns(blk), "null_allocator thinks it doesn't own the block it created.");
	na.deallocate(blk);
}

void mallocator_test()
{
	tz::mallocator ma;
	tz::memblk blk = ma.allocate(1024);
	tz::assert(blk != tz::nullblk, "mallocator failed to alloc 1024 bytes.");
	tz::assert(ma.owns(blk), "mallocator thinks it does not own the block it created.");

	std::vector<int, tz::allocator_adapter<int, tz::mallocator>> ints{1, 2, 3, 4};
	ints.clear();
	ints.push_back(5);

	std::deque<int, tz::allocator_adapter<int, tz::mallocator>> int_deque;
	int_deque.push_back(0);
	int_deque.push_front(1);
	int_deque.clear();
	int_deque.resize(10);
	std::iota(int_deque.begin(), int_deque.end(), 0);
}

/*
void stack_allocator_tests()
{
	tz::stack_allocator<64> s;
	auto blk2 = s.allocate(1);
	tz::assert(s.owns(blk2) && !s.owns(tz::nullblk), "stack_allocator says it doesn't own a block it allocated, or thinks it owns nullblk");
	s.deallocate(blk2);

	using LocalAllocator = tz::stack_allocator<1024>;
	std::vector<int, tz::allocator_adapter<int, LocalAllocator>> ints2(0);
	ints2.resize(4);
	ints2.push_back(5);
	ints2.push_back(6);
}
*/

void linear_allocator_tests()
{
	// First create some storage we can use.
	tz::mallocator mallocator;
	tz::memblk scratch = mallocator.allocate(256);
	{
		tz::linear_allocator lalloc{scratch};
		tz::memblk blk = lalloc.allocate(256);
		tz::assert(lalloc.owns(blk), "linear_allocator allocated its entire contents, but says it doesn't own the resultant block");
		lalloc.deallocate(blk);
		tz::assert((blk = lalloc.allocate(1)) != tz::nullblk, "linear_allocator thinks it ran out of space but I deallocated it to empty, the next allocation returned the null block.");
		lalloc.deallocate(blk);
		tz::assert(lalloc.allocate(257) == tz::nullblk, "linear_allocator ran out of space, but didn't output the null block.");

		// The adapter linear allocator will happily overwrite any previous allocations.
		tz::allocator_adapter<int, tz::linear_allocator> int_lalloc{lalloc};
		std::vector<int, tz::allocator_adapter<int, tz::linear_allocator>> ints{int_lalloc};
		ints.reserve(4);
		ints.push_back(1);
		ints.push_back(2);

		// Check memory values
		tz::assert(*(reinterpret_cast<int*>(scratch.ptr) + 0) == 1, "linear_allocator is allocating in a dodgy area in its arena.");
		tz::assert(*(reinterpret_cast<int*>(scratch.ptr) + 1) == 2, "linear_allocator is allocating in a dodgy area in its arena.");

		ints.clear();
		ints.push_back(3);
		tz::assert(*(reinterpret_cast<int*>(scratch.ptr) + 0) == 3, "linear_allocator is allocating in a dodgy area in its arena (post-dealloc)");
	}
	mallocator.deallocate(scratch);
}

void slab_allocator_tests()
{
	tz::slab_allocator salloc;
	for(std::size_t i = 0; i < 4096u; i++)
	{
		tz::memblk blk = salloc.allocate(9182u);
		tz::assert(salloc.owns(blk));
	}
}

int main()
{
	null_allocator_tests();
	mallocator_test();
	//stack_allocator_tests();
	linear_allocator_tests();
	slab_allocator_tests();
}
