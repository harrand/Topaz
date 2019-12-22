//
// Created by Harrand on 21/12/2019.
//

#include "test_framework.hpp"
#include "memory/pool.hpp"

tz::test::Case uniform()
{
	tz::test::Case test_case("tz::mem::UniformPool Basic Tests");
	// allocate some memory
	constexpr std::size_t ele_size = 8; // number of elements we can store.
	using StorageType = std::aligned_storage_t<sizeof(int), alignof(int)>;
	constexpr std::size_t mem_size = ele_size * sizeof(StorageType);
	StorageType mem[ele_size];
	tz::mem::UniformPool<int> ints{&mem, mem_size};
	
	topaz_expect(test_case, ints.capacity_bytes() == mem_size, "UniformPool<int> has unexpected byte capacity upon construction: ", ints.capacity_bytes());
	topaz_expect(test_case, ints.empty(), "Uniform<int> fresh container was wrongly considered to be non-empty. Size == ", ints.size());
	topaz_expect(test_case, ints.capacity() == ele_size, "UniformPool<int> had unexpected capacity upon construction: ", ints.capacity());
	topaz_expect(test_case, ints.size() == 0, "UniformPool<int> has unexpected size upon construction: ", ints.size());
	
	ints.set(0, 5);
	topaz_expect(test_case, ints.size() == 1, "UniformPool<int> had unexpected size after setting an element: ", ints.size());
	topaz_expect(test_case, ints.capacity() == ele_size, "UniformPool<int> had unexpected capacity after setting an element: ", ints.capacity());
	topaz_expect(test_case, ints[0] == 5, "UniformPool<int> failed to set value to initially unset location at index ", 0);
	
	// Should definitely no longer be empty.
	topaz_expect(test_case, !ints.empty(), "UniformPool<T> wrongly considered to be empty after setting an element");
	
	ints.set(0, 50);
	topaz_expect(test_case, ints.size() == 1, "UniformPool<int> had unexpected size after setting an element: ", ints.size());
	topaz_expect(test_case, ints.capacity() == ele_size, "UniformPool<int> had unexpected capacity after setting an element: ", ints.capacity());
	topaz_expect(test_case, ints[0] == 50, "UniformPool<int> failed to set value to initially unset location at index ", 0);
	
	ints.set(1, 12);
	topaz_expect(test_case, ints.size() == 2, "UniformPool<int> had unexpected size after setting an element: ", ints.size());
	topaz_expect(test_case, ints.capacity() == ele_size, "UniformPool<int> had unexpected capacity after setting an element: ", ints.capacity());
	topaz_expect(test_case, ints[1] == 12, "UniformPool<int> failed to set value to initially unset location at index ", 1);
	
	ints.set(2, 99);
	topaz_expect(test_case, ints.size() == 3, "UniformPool<int> had unexpected size after setting an element: ", ints.size());
	topaz_expect(test_case, ints.capacity() == ele_size, "UniformPool<int> had unexpected capacity after setting an element: ", ints.capacity());
	topaz_expect(test_case, ints[2] == 99, "UniformPool<int> failed to set value to initially unset location at index ", 2);
	
	ints.set(3, 45);
	topaz_expect(test_case, ints.size() == 4, "UniformPool<int> had unexpected size after setting an element: ", ints.size());
	topaz_expect(test_case, ints.capacity() == ele_size, "UniformPool<int> had unexpected capacity after setting an element: ", ints.capacity());
	topaz_expect(test_case, ints[3] == 45, "UniformPool<int> failed to set value to initially unset location at index ", 3);
	
	ints.set(4, 11);
	topaz_expect(test_case, ints.size() == 5, "UniformPool<int> had unexpected size after setting an element: ", ints.size());
	topaz_expect(test_case, ints.capacity() == ele_size, "UniformPool<int> had unexpected capacity after setting an element: ", ints.capacity());
	topaz_expect(test_case, ints[4] == 11, "UniformPool<int> failed to set value to initially unset location at index ", 4);
	
	ints.set(5, 10);
	topaz_expect(test_case, ints.size() == 6, "UniformPool<int> had unexpected size after setting an element: ", ints.size());
	topaz_expect(test_case, ints.capacity() == ele_size, "UniformPool<int> had unexpected capacity after setting an element: ", ints.capacity());
	topaz_expect(test_case, ints[5] == 10, "UniformPool<int> failed to set value to initially unset location at index ", 5);
	
	ints.set(6, 69);
	topaz_expect(test_case, ints.size() == 7, "UniformPool<int> had unexpected size after setting an element: ", ints.size());
	topaz_expect(test_case, ints.capacity() == ele_size, "UniformPool<int> had unexpected capacity after setting an element: ", ints.capacity());
	topaz_expect(test_case, ints[6] == 69, "UniformPool<int> failed to set value to initially unset location at index ", 6);
	
	// We're not quite full yet.
	topaz_expect(test_case, !ints.full(), "UniformPool<int> wrongly considered to be full. Capacity && Size == ", ints.size());
	
	ints.set(7, 83);
	topaz_expect(test_case, ints.size() == 8, "UniformPool<int> had unexpected size after setting an element: ", ints.size());
	topaz_expect(test_case, ints.capacity() == ele_size, "UniformPool<int> had unexpected capacity after setting an element: ", ints.capacity());
	topaz_expect(test_case, ints[7] == 83, "UniformPool<int> failed to set value to initially unset location at index ", 7);
	
	// Now we're full!
	topaz_expect(test_case, ints.full(), "UniformPool<int> wrongly considered to not be full. Capacity == ", ints.capacity(), ", Size == ", ints.size());
	
	// Let's erase an element.
	ints.erase(7);
	topaz_expect(test_case, ints.size() == 7, "UniformPool<int> failed to erase the final element at index", 7, " -- Size = ", ints.size());
	topaz_expect(test_case, ints.size() == 7, "UniformPool<int> had unexpected size after erasing an element: ", ints.size());
	topaz_expect(test_case, ints.capacity() == ele_size, "UniformPool<int> had unexpected capacity after erasing an element: ", ints.capacity());
	// Not full anymore!
	topaz_expect(test_case, !ints.full(), "UniformPool<int> wrongly considered to be full. Capacity && Size == ", ints.size());
	
	// Aaaand we'll reset it to a new value.
	ints.set(7, 9000);
	topaz_expect(test_case, ints.size() == 8, "UniformPool<int> had unexpected size after setting an element: ", ints.size());
	topaz_expect(test_case, ints.capacity() == ele_size, "UniformPool<int> had unexpected capacity after setting an element: ", ints.capacity());
	topaz_expect(test_case, ints[7] == 9000, "UniformPool<int> failed to set value to initially unset location at index ", 7);
	// Full again!
	topaz_expect(test_case, ints.full(), "UniformPool<int> wrongly considered to not be full. Capacity == ", ints.capacity(), ", Size == ", ints.size());
	
	ints.clear();
	// Should be empty now.
	topaz_expect(test_case, ints.size() == 0, "UniformPool<int>'s clear() operation failed to make the size zero. Size == ", ints.size());
	topaz_expect(test_case, ints.empty(), "UniformPool<int>'s clear() operation failed to make the container empty. Size == ", ints.size());
	
	// TODO: Remove
	ints.debug_print_as<int>();
	
	return test_case;
}

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

tz::test::Case object_semantics()
{
	tz::test::Case test_case("tz::mem::UniformPool Object Tests");
	// allocate some memory
	constexpr std::size_t ele_size = 8; // number of elements we can store.
	using StorageType = std::aligned_storage_t<sizeof(Reference), alignof(Reference)>;
	constexpr std::size_t mem_size = ele_size * sizeof(StorageType);
	StorageType mem[ele_size];
	tz::mem::UniformPool<Reference> refs{&mem, mem_size};
	
	std::size_t count = 0;
	refs.set(0, Reference{count});
	topaz_expect(test_case, count == 1, "UniformPool<Reference> failed to increment reference count (Constructor semantics).");
	refs.erase(0);
	topaz_expect(test_case, count == 0, "UniformPool<Reference> failed to decrement reference count (Destructor semantics).");
	refs.set(0, Reference{count});
	refs.set(1, Reference{count});
	refs.set(2, Reference{count});
	topaz_expect(test_case, count == 3, "UniformPool<Reference> did not produce expected refcount. Expected 3, got ", count);
	refs.clear();
	refs.debug_print_as<std::size_t>();
	topaz_expect(test_case, count == 0, "UniformPool<Reference> did not produce expected refcount. Expected 0, got ", count);
	return test_case;
}

tz::test::Case overflow()
{
	tz::test::Case test_case("tz::mem::UniformPool Overflow Tests");
	
	return test_case;
}

int main()
{
	tz::test::Unit pool;
	pool.add(uniform());
	pool.add(object_semantics());
	pool.add(overflow());
	
	return pool.result();
}