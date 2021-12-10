#include "core/allocators/linear.hpp"
#include "core/assert.hpp"
#include <vector>
#include <numeric>

template<typename T>
using LinearVector = std::vector<T, tz::LinearAllocator<T>>;

void linear_alloc()
{
	// Firstly create a memory resource for the allocator to use.
	std::array<std::byte, 128> buffer;
	std::fill(buffer.begin(), buffer.end(), std::byte{0});
	
	tz::LinearAllocator<int> int_alloc{buffer.data(), buffer.size()};
	LinearVector<int> vec(int_alloc);

	// add 1, 2, 3, 4, 5... to the vec, enough to completely fill the buffer.
	vec.resize(128 / sizeof(int));
	std::iota(vec.begin(), vec.end(), 0);

	std::span<const int> buf_view_ints{reinterpret_cast<const int*>(buffer.data()), buffer.size() / sizeof(int)};
	tz_assert(std::equal(buf_view_ints.begin(), buf_view_ints.end(), vec.begin(), vec.end()), "Reserved vector did not directly match the buffer");

}

int main()
{
	linear_alloc();
}
