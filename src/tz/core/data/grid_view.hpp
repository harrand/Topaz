#ifndef TOPAZ_CORE_CONTAINERS_GRID_VIEW_HPP
#define TOPAZ_CORE_CONTAINERS_GRID_VIEW_HPP
#include "tz/core/data/vector.hpp"
#include <span>
#include <functional>

namespace tz
{
	/**
	 * @ingroup tz_core_data
	 * A view into an array representing a flattened grid of data.
	 *
	 * Elements are interpreted as a tightly-packed array of rows. Each element is expected to be equivalent to a T[N]. For example, grid_view on an RGBA32 image might be a `grid_view<std::byte, 4>`.
	 * @tparam T Underlying type of the element.
	 * @tparam N number of T's per element. Default 1.
	 */
	template<typename T, std::size_t N = 1>
	class grid_view
	{
	public:
		/**
		 * Create a view over an existing array of grid data.
		 * @param data View over grid data.
		 * @param dimensions {x, y} where x is number of columns, and y is number of rows.
		 */
		grid_view(std::span<T> data, tz::vec2ui dimensions);
		/**
		 * Create a view over an existing array of square grid data.
		 * @param data View over grid data.
		 * @param length Represents the number of columns and rows.
		 */
		grid_view(std::span<T> data, unsigned int length);
		/**
		 * Retrieve the dimensions of the grid, in elements.
		 * @return {width, height}.
		 */
		tz::vec2ui get_dimensions() const;
		/**
		 * Retrieve the underlying span.
		 */
		std::span<T> span();
		/**
		 * Retrieve the underlying span. Read-only.
		 */
		std::span<const T> span() const;

		/**
		 * Retrieve the element at the provided co-ordinate.
		 *
		 * Note that normally this returns a span representing the T[] of an element. However if N == 1, then a reference to a single T is retrieved instead.
		 * @param x Column index locating the element.
		 * @param y Row index locating the element.
		 * @return Reference to the element, via a normal ref (if an element has only one T) or a span<T> of size equal to N.
		 */
		std::conditional_t<N == 1, T&, std::span<T>> operator()(unsigned int x, unsigned int y);
	private:
		std::span<T> data;
		tz::vec2ui dimensions;
	};
}

#include "tz/core/data/grid_view.inl"

#endif // TOPAZ_CORE_CONTAINERS_GRID_VIEW_HPP
