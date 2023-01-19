#include "tz/core/debug.hpp"

namespace tz
{
	template<typename T, std::size_t N>
	GridView<T, N>::GridView(std::span<T> data, tz::vec2ui dimensions):
	data(data),
	dimensions(dimensions)
	{
		if(this->data.size_bytes() < (this->dimensions[0] * this->dimensions[1] * sizeof(T) * N))
		{
			tz::report("GridView passed a span of unexpected size. View is %ux%u where each element is %zu bytes, for a expected buffer size of %zu, but buffer provided has %zu bytes.", this->dimensions[0], this->dimensions[1], sizeof(T) * N, (this->dimensions[0] * this->dimensions[1] * sizeof(T) * N), this->data.size_bytes());
		}
	}

	template<typename T, std::size_t N>
	GridView<T, N>::GridView(std::span<T> data, unsigned int length):
	GridView<T, N>(data, {length, length}){}

	template<typename T, std::size_t N>
	tz::vec2ui GridView<T, N>::get_dimensions() const
	{
		return this->dimensions;
	}

	template<typename T, std::size_t N>
	std::span<T> GridView<T, N>::span()
	{
		return this->data;
	}

	template<typename T, std::size_t N>
	std::span<const T> GridView<T, N>::span() const
	{
		return this->data;
	}

	template<typename T, std::size_t N>
	std::conditional_t<N == 1, T&, std::span<T>> GridView<T, N>::operator()(unsigned int x, unsigned int y)
	{
		std::size_t idx = ((y * this->dimensions[0]) + x) * N;
		tz::assert(x < this->dimensions[0] && y < this->dimensions[1], "GridView access out of bounds. Dimensions = {%u, %u} must be > {%u, %u} which was requested", this->dimensions[0], this->dimensions[1], x, y);
		tz::assert(idx < this->data.size(), "GridView access out of bounds. About to invoke UB.");
		if constexpr(N == 1)
		{
			return this->data[idx];
		}
		else
		{
			return this->data.subspan(idx, N);
		}
	}
}
