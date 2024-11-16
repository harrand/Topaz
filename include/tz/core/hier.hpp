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
	/**
	 * @ingroup tz_core_hier
	 * @brief Destroy an existing hierarchy, along with all of its nodes.
	 */
	void destroy_hier(hier_handle hier);
	/**
	 * @ingroup tz_core_hier
	 * @brief Create a new node within the hierarchy.
	 *
	 * @param hier Hierarchy in which to create the node.
	 * @param transform Initial local transform of the node.
	 * @param parent Handle to a node which should be this new node's parent.
	 * @param userdata User data to attach to the node.
	 * @return @ref tz::error_code::invalid_value If `parent` is not a null handle, but also doesn't refer to a valid node within the hierarchy. For example, the node handle is invalid or has been destroyed.
	 */
	std::expected<node_handle, tz::error_code> hier_create_node(hier_handle hier, tz::trs transform = {}, node_handle parent = tz::nullhand, void* userdata = nullptr);
	/**
	 * @ingroup tz_core_hier
	 * @brief Destroy a node within the hierarchy.
	 *
	 * Any children are also destroyed.
	 *
	 * @param hier Hierarchy in which the node is located. This must be the same handle used in the call to @ref hier_create_node.
	 * @param node Node to destroy.
	 * @return @ref tz::error_code::invalid_value If `node` is invalid or has already been destroyed.
	 * @return @ref tz::error_code::unknown_error If destruction of a child node failed.
	 */
	tz::error_code hier_destroy_node(hier_handle hier, node_handle node);
	
	/**
	 * @ingroup tz_core_hier
	 * @brief Retrieve the local transform of a node.
	 *
	 * @return @ref tz::error_code::invalid_value If `hier` or `node` are invalid or have previously been destroyed.
	 */
	std::expected<tz::trs, tz::error_code> hier_node_get_local_transform(hier_handle hier, node_handle node);
	/**
	 * @ingroup tz_core_hier
	 * @brief Set the local transform of a node.
	 */
	void hier_node_set_local_transform(hier_handle hier, node_handle node, tz::trs transform);
	/**
	 * @ingroup tz_core_hier
	 * @brief Retrieve the global transform of a node.
	 *
	 * @return @ref tz::error_code::invalid_value If `hier` or `node` are invalid or have previously been destroyed.
	 */
	std::expected<tz::trs, tz::error_code> hier_node_get_global_transform(hier_handle hier, node_handle node);
	/**
	 * @ingroup tz_core_hier
	 * @brief Set the global transform of a node.
	 */
	void hier_node_set_global_transform(hier_handle hier, node_handle node, tz::trs transform);
}

#endif // TOPAZ_CORE_HIER_HPP