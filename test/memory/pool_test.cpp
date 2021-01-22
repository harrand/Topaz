//
// Created by Harrand on 21/12/2019.
//

#include "test_framework.hpp"
#include "memory/pool.hpp"

TZ_TEST_BEGIN(uniform)
	// allocate some memory
	constexpr std::size_t ele_size = 8; // number of elements we can store.
	using StorageType = std::aligned_storage_t<sizeof(int), alignof(int)>;
	constexpr std::size_t mem_size = ele_size * sizeof(StorageType);
	StorageType mem[ele_size];
	tz::mem::UniformPool<int> ints{&mem, mem_size};
	
	topaz_expect(ints.capacity_bytes() == mem_size, "UniformPool<int> has unexpected byte capacity upon construction: ", ints.capacity_bytes());
	topaz_expect(ints.empty(), "Uniform<int> fresh container was wrongly considered to be non-empty. Size == ", ints.size());
	topaz_expect(ints.capacity() == ele_size, "UniformPool<int> had unexpected capacity upon construction: ", ints.capacity());
	topaz_expect(ints.size() == 0, "UniformPool<int> has unexpected size upon construction: ", ints.size());
	
	ints.set(0, 5);
	topaz_expect(ints.size() == 1, "UniformPool<int> had unexpected size after setting an element: ", ints.size());
	topaz_expect(ints.capacity() == ele_size, "UniformPool<int> had unexpected capacity after setting an element: ", ints.capacity());
	topaz_expect(ints[0] == 5, "UniformPool<int> failed to set value to initially unset location at index ", 0);
	
	// Should definitely no longer be empty.
	topaz_expect(!ints.empty(), "UniformPool<T> wrongly considered to be empty after setting an element");
	
	ints.set(0, 50);
	topaz_expect(ints.size() == 1, "UniformPool<int> had unexpected size after setting an element: ", ints.size());
	topaz_expect(ints.capacity() == ele_size, "UniformPool<int> had unexpected capacity after setting an element: ", ints.capacity());
	topaz_expect(ints[0] == 50, "UniformPool<int> failed to set value to initially unset location at index ", 0);
	
	ints.set(1, 12);
	topaz_expect(ints.size() == 2, "UniformPool<int> had unexpected size after setting an element: ", ints.size());
	topaz_expect(ints.capacity() == ele_size, "UniformPool<int> had unexpected capacity after setting an element: ", ints.capacity());
	topaz_expect(ints[1] == 12, "UniformPool<int> failed to set value to initially unset location at index ", 1);
	
	ints.set(2, 99);
	topaz_expect(ints.size() == 3, "UniformPool<int> had unexpected size after setting an element: ", ints.size());
	topaz_expect(ints.capacity() == ele_size, "UniformPool<int> had unexpected capacity after setting an element: ", ints.capacity());
	topaz_expect(ints[2] == 99, "UniformPool<int> failed to set value to initially unset location at index ", 2);
	
	ints.set(3, 45);
	topaz_expect(ints.size() == 4, "UniformPool<int> had unexpected size after setting an element: ", ints.size());
	topaz_expect(ints.capacity() == ele_size, "UniformPool<int> had unexpected capacity after setting an element: ", ints.capacity());
	topaz_expect(ints[3] == 45, "UniformPool<int> failed to set value to initially unset location at index ", 3);
	
	ints.set(4, 11);
	topaz_expect(ints.size() == 5, "UniformPool<int> had unexpected size after setting an element: ", ints.size());
	topaz_expect(ints.capacity() == ele_size, "UniformPool<int> had unexpected capacity after setting an element: ", ints.capacity());
	topaz_expect(ints[4] == 11, "UniformPool<int> failed to set value to initially unset location at index ", 4);
	
	ints.set(5, 10);
	topaz_expect(ints.size() == 6, "UniformPool<int> had unexpected size after setting an element: ", ints.size());
	topaz_expect(ints.capacity() == ele_size, "UniformPool<int> had unexpected capacity after setting an element: ", ints.capacity());
	topaz_expect(ints[5] == 10, "UniformPool<int> failed to set value to initially unset location at index ", 5);
	
	ints.set(6, 69);
	topaz_expect(ints.size() == 7, "UniformPool<int> had unexpected size after setting an element: ", ints.size());
	topaz_expect(ints.capacity() == ele_size, "UniformPool<int> had unexpected capacity after setting an element: ", ints.capacity());
	topaz_expect(ints[6] == 69, "UniformPool<int> failed to set value to initially unset location at index ", 6);
	
	// We're not quite full yet.
	topaz_expect(!ints.full(), "UniformPool<int> wrongly considered to be full. Capacity && Size == ", ints.size());
	
	ints.set(7, 83);
	topaz_expect(ints.size() == 8, "UniformPool<int> had unexpected size after setting an element: ", ints.size());
	topaz_expect(ints.capacity() == ele_size, "UniformPool<int> had unexpected capacity after setting an element: ", ints.capacity());
	topaz_expect(ints[7] == 83, "UniformPool<int> failed to set value to initially unset location at index ", 7);
	
	// Now we're full!
	topaz_expect(ints.full(), "UniformPool<int> wrongly considered to not be full. Capacity == ", ints.capacity(), ", Size == ", ints.size());
	
	// Let's erase an element.
	ints.erase(7);
	topaz_expect(ints.size() == 7, "UniformPool<int> failed to erase the final element at index", 7, " -- Size = ", ints.size());
	topaz_expect(ints.size() == 7, "UniformPool<int> had unexpected size after erasing an element: ", ints.size());
	topaz_expect(ints.capacity() == ele_size, "UniformPool<int> had unexpected capacity after erasing an element: ", ints.capacity());
	// Not full anymore!
	topaz_expect(!ints.full(), "UniformPool<int> wrongly considered to be full. Capacity && Size == ", ints.size());
	
	// Aaaand we'll reset it to a new value.
	ints.set(7, 9000);
	topaz_expect(ints.size() == 8, "UniformPool<int> had unexpected size after setting an element: ", ints.size());
	topaz_expect(ints.capacity() == ele_size, "UniformPool<int> had unexpected capacity after setting an element: ", ints.capacity());
	topaz_expect(ints[7] == 9000, "UniformPool<int> failed to set value to initially unset location at index ", 7);
	// Full again!
	topaz_expect(ints.full(), "UniformPool<int> wrongly considered to not be full. Capacity == ", ints.capacity(), ", Size == ", ints.size());
	
	ints.clear();
	// Should be empty now.
	topaz_expect(ints.size() == 0, "UniformPool<int>'s clear() operation failed to make the size zero. Size == ", ints.size());
	topaz_expect(ints.empty(), "UniformPool<int>'s clear() operation failed to make the container empty. Size == ", ints.size());
TZ_TEST_END

struct Reference
{
public:
	Reference(std::size_t& ref): ref(ref)
	{
		this->ref++;
	}
	
	Reference(const Reference& copy): Reference(copy.ref){}
	
	Reference(Reference&& move): Reference(move.ref){}
	
	~Reference()
	{
		this->ref--;
	}
private:
	std::size_t& ref;
};

TZ_TEST_BEGIN(object_semantics)
	// allocate some memory
	constexpr std::size_t ele_size = 8; // number of elements we can store.
	using StorageType = std::aligned_storage_t<sizeof(Reference), alignof(Reference)>;
	constexpr std::size_t mem_size = ele_size * sizeof(StorageType);
	StorageType mem[ele_size];
	tz::mem::UniformPool<Reference> refs{&mem, mem_size};
	
	std::size_t count = 0;
	refs.set(0, Reference{count});
	topaz_expect(count == 1, "UniformPool<Reference> failed to increment reference count (Constructor semantics).");
	refs.erase(0);
	topaz_expect(count == 0, "UniformPool<Reference> failed to decrement reference count (Destructor semantics).");
	refs.set(0, Reference{count});
	refs.set(1, Reference{count});
	refs.set(2, Reference{count});
	topaz_expect(count == 3, "UniformPool<Reference> did not produce expected refcount. Expected 3, got ", count);
	refs.clear();
	topaz_expect(count == 0, "UniformPool<Reference> did not produce expected refcount. Expected 0, got ", count);
TZ_TEST_END

TZ_TEST_BEGIN(overflow)
	constexpr std::size_t ele_size = 2; // number of elements we can store.
	using StorageType = std::aligned_storage_t<sizeof(int), alignof(int)>;
	constexpr std::size_t mem_size = ele_size * sizeof(int);
	StorageType mem[ele_size];
	
	tz::mem::UniformPool<int> ints{&mem, mem_size};
	ints.set(0, 5);
	ints.set(1, 6);
	// We expect to assert on an overflow.
	topaz_expect_assert(false, "UniformPool<int> asserted sooner than expected.");
	ints.set(2, 7);
	topaz_expect_assert(true, "UniformPool<int> didn't assert when expected (overflowed pool)");
	topaz_assert_clear();
TZ_TEST_END

TZ_TEST_BEGIN(statics)
	using Pool = tz::mem::StaticPool<int, float, std::string>;
	tz::mem::AutoBlock blk{Pool::size()};

	{
		// Pre-initialised from pointer.
		Pool spool{blk.begin, 5, 6.0f, "well met!"};

		int a = spool.get<0>();
		topaz_expect(a == 5, "tz::mem::StaticPool<...>::get<0>(): Expected int value ", 5, ", but got ", a);

		float b = spool.get<1>();
		topaz_expect(b == 6.0f, "tz::mem::StaticPool<...>::get<1>(): Expected float value ", 6.0f, ", but got ", b);

		std::string c = spool.get<2>();
		topaz_expect(c == "well met!", "tz::mem::StaticPool<...>::get<2>(): Expected string value \"", "well met!", "\", but got \"", c, "\"");
	}

	{
		// Uninitialised from a block
		Pool spool{blk};

		spool.set<0>(15);
		std::size_t actual = spool.get<0>();
		topaz_expect(actual == 15, "tz::mem::StaticPool<...>::get<0>(): Invalid value after invoking set<0>: Set to ", 15, ", but following get returned ", actual);
	}
TZ_TEST_END

int main()
{
	tz::test::Unit pool;
	pool.add(uniform());
	pool.add(object_semantics());
	pool.add(overflow());
	pool.add(statics());
	
	return pool.result();
}