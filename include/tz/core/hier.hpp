#ifndef TOPAZ_CORE_HIER_HPP
#define TOPAZ_CORE_HIER_HPP
#include "tz/core/error.hpp"
#include "tz/core/trs.hpp"
#include "tz/core/handle.hpp"
#include <expected>

namespace tz
{
	/**
	 * @ingroup tz_core
	 * @defgroup tz_core_hier Hierarchies
	 * @brief Create hierarchies of transforms with attached data.
	 */

	namespace detail{struct hier_t{};}
	/**
	 * @ingroup tz_core_hier
	 * @brief Represents a single hierarchy.
	 *
	 * See @ref create_hier for details.
	 */
	using hier_handle = tz::handle<detail::hier_t>;
	/**
	 * @ingroup tz_core_hier
	 * @brief Represents a single node within a hierarchy.
	 */
	using node_handle = tz::handle<hier_handle>;

	/**
	 * @ingroup tz_core_hier
	 * @brief Create a new empty hierarchy.
	 */
	hier_handle create_hier();
	std::expected<node_handle, tz::error_code> hier_create_node(hier_handle hier, tz::trs transform = {}, node_handle parent = tz::nullhand, void* userdata = nullptr);
	tz::error_code hier_destroy_node(hier_handle hier, node_handle node);
}

#endif // TOPAZ_CORE_HIER_HPP