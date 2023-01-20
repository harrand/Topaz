#include "tz/core/debug.hpp"

namespace tz
{
	template<typename T, std::size_t N>
	grid_view<T, N>::grid_view(std::span<T> data, tz::vec2ui dimensions):
	data(data),
	dimensions(dimensions)
	{
		if(this->data.size_bytes() < (this->dimensions[0] * this->dimensions[1] * sizeof(T) * N))
		{
			tz::report("grid_view passed a span of unexpected size. View is %ux%u where each element is %zu bytes, for a expected buffer size of %zu, but buffer provided has %zu bytes.", this->dimensions[0], this->dimensions[1], sizeof(T) * N, (this->dimensions[0] * this->dimensions[1] * sizeof(T) * N), this->data.size_bytes());
		}
	}

	template<typename T, std::size_t N>
	grid_view<T, N>::grid_view(std::span<T> data, unsigned int length):
	grid_view<T, N>(data, {length, length}){}

	template<typename T, std::size_t N>
	tz::vec2ui grid_view<T, N>::get_dimensions() const
	{
		return this->dimensions;
	}

	template<typename T, std::size_t N>
	std::span<T> grid_view<T, N>::span()
	{
		return this->data;
	}

	template<typename T, std::size_t N>
	std::span<const T> grid_view<T, N>::span() const
	{
		return this->data;
	}

	template<typename T, std::size_t N>
	std::conditional_t<N == 1, T&, std::span<T>> grid_view<T, N>::operator()(unsigned int x, unsigned int y)
	{
		std::size_t idx = ((y * this->dimensions[0]) + x) * N;
		tz::assert(x < this->dimensions[0] && y < this->dimensions[1], "grid_view access out of bounds. Dimensions = {%u, %u} must be > {%u, %u} which was requested", this->dimensions[0], this->dimensions[1], x, y);
		tz::assert(idx < this->data.size(), "grid_view access out of bounds. About to invoke UB.");
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
