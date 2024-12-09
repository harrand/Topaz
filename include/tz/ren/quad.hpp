#ifndef TOPAZ_REN_QUAD_HPP
#define TOPAZ_REN_QUAD_HPP
#include "tz/core/handle.hpp"
#include "tz/core/vector.hpp"
#include "tz/gpu/graph.hpp"

namespace tz::ren
{
	namespace detail
	{
		struct quad_t{};
		struct quadren_t{};
	}
	/**
	 * @ingroup tz_ren
	 * @defgroup tz_ren_quad Quad Renderer
	 * @brief Efficiently render a large number of 2D quads.
	 *
	 * 1. Create a quad renderer via @ref create_quad_renderer.
	 * 2. Create a bunch of quads using repeated calls to @ref quad_renderer_create_quad.
	 * 3. Every frame, retrieve the graph via @ref quad_renderer_graph and pass that to @ref tz::gpu::execute to render all the quads.
	 */

	/**
	 * @ingroup tz_ren_quad
	 * @brief Represents a single quad. Owned by a @ref quad_renderer_handle.
	 */
	using quad_handle = tz::handle<detail::quad_t>;
	/**
	 * @ingroup tz_ren_quad
	 * @brief Represents a single quad renderer instance.
	 *
	 * You can have multiple quad renderers at the same time. Any @ref quad_handle retrieved from creating a quad will be owned solely by whichever quad renderer you used to create it.
	 */
	using quad_renderer_handle = tz::handle<detail::quadren_t>;

	/**
	 * @ingroup tz_ren_quad
	 * @brief Specifies optional, extra functionality for a quad renderer.
	 */
	enum quad_renderer_flag : std::int64_t
	{
		/// If the alpha-component of any fragment in a quad (after texture sampling) is very low (<0.05f), then that fragment will be discarded.
		alpha_clipping = 0b00000001,
		/// Sets @ref tz::gpu::graph_flag::present_after on the graph representing the quad renderer.
		graph_present_after = 0b00000010,
		/// Normally if a quad has a negative scale in any dimension, the triangles are no longer in the correct winding order and will thus be invisible. Setting this flags disables face culling, meaning triangles scaled negatively (and thus in the wrong winding order) will still display as normal.
		allow_negative_scale = 0b00000100,
		/// Enables the use of the layer property of a quad. By default, everything is on layer 0. Layer values are between -100 and 100. A quad with a higher layer will be drawn over a quad with a lower layer.
		enable_layering = 0b00001000,
		/// Enables the use of @ref quad_renderer_info::custom_fragment_shader.
		custom_fragment_shader = 0b00010000,
	};

	constexpr quad_renderer_flag operator|(quad_renderer_flag lhs, quad_renderer_flag rhs)
	{
		return static_cast<quad_renderer_flag>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}

	constexpr bool operator&(quad_renderer_flag lhs, quad_renderer_flag& rhs)
	{
		return static_cast<int>(lhs) & static_cast<int>(rhs);
	}

	/**
	 * @ingroup tz_ren_quad
	 * @brief Specifies creation flags for a quad renderer.
	 *
	 * See @ref create_quad_renderer for usage.
	 */
	struct quad_renderer_info
	{
		/// When the main pass is executed, the colour target will be cleared to this colour value.
		tz::v4f clear_colour = {0.0f, 0.0f, 0.0f, 1.0f};
		/// Specifies the colour target to render into. By default, this is the window resource (i.e the quad renderer will draw to the window unless you specify a different colour target).
		std::span<const tz::gpu::resource_handle> colour_targets =
			[]()->std::span<const tz::gpu::resource_handle>
			{
				static tz::gpu::resource_handle default_targets[1];
				default_targets[0] = tz::gpu::window_resource;
				return {std::begin(default_targets), std::end(default_targets)};
			}();
		std::span<const tz::gpu::resource_handle> extra_resources = {};
		tz::gpu::graph_handle post_render = tz::nullhand;
		/// Any extra optional flags to specify?
		quad_renderer_flag flags = static_cast<quad_renderer_flag>(0);
		/// Custom fragment shader. Unless @ref quad_renderer_flag::custom_fragment_shader is specified, this value is ignored and a default fragment shader is used.
		std::string_view custom_fragment_shader = {};
		/// How much extraneous GPU memory should be allocated per quad? If you want to store extra data per quad, you should put the size of that data here.
		std::size_t extra_data_per_quad = 0;
	};
	
	/**
	 * @ingroup tz_ren_quad
	 * @brief Create a new quad renderer.
	 * @return On success: A @ref quad_renderer_handle corresponding to the newly-created quad renderer.
	 * @return On failure: Any error returned by the automatic call to @ref create_pass.
	 */
	std::expected<quad_renderer_handle, tz::error_code> create_quad_renderer(quad_renderer_info info);
	/**
	 * @ingroup tz_ren_quad
	 * @brief Manually destroy a quad renderer.
	 * @return On failure: Any error returned by the automatic calls to @ref destroy_resource.
	 */
	tz::error_code destroy_quad_renderer(quad_renderer_handle renh);

	/**
	 * @ingroup tz_ren_quad
	 * @brief Specifies initial data for a single quad.
	 */
	struct quad_info
	{
		/// Position of the quad, in world-space. You can change this later via @ref set_quad_position.
		tz::v2f position = tz::v2f::zero();
		/// Rotation of the quads, in radians.
		float rotation = 0.0f;
		/// Scale factors of the quad, in both dimensions. You can change this later via @ref set_quad_scale.
		tz::v2f scale = tz::v2f::filled(1.0f);
		/// First texture to display on the quad. Defaults to -1 (no texture). If no texture is used, then the quad will have a solid colour corresponding to @ref colour. You can change this later via @ref set_quad_texture0.
		std::uint32_t texture_id0 = -1;
		/// First texture to display on the quad. Defaults to -1 (no texture). If no texture is used, then the quad will have a solid colour corresponding to @ref colour. You can change this later via @ref set_quad_texture1.
		std::uint32_t texture_id1 = -1;
		/// Colour of the quad. If the quad has no texture, this will be the exact colour of the whole quad. If the quad *does* have a texture, then the sampled texture colour will be multiplied by this value (in which case you will often want to provide {1, 1, 1}). You can change this later via @ref set_quad_colour.
		tz::v3f colour = tz::v3f::filled(1.0f);
		/// Layer value. Has no effect if layering is not enabled (see @ref quad_renderer_flag::enable_layering for more details).
		short layer = 0;
	};
	
	/**
	 * @ingroup tz_ren_quad
	 * @brief Create a new quad to be rendered by an existing quad renderer.
	 * @return On success, a handle corresponding to the newly created quad.
	 * @return @ref tz::error_code::invalid_value If you provide an invalid texture-id.
	 *
	 * Next time the quad renderer executes, this new quad will be visible with the info provided.
	 */
	std::expected<quad_handle, tz::error_code> quad_renderer_create_quad(quad_renderer_handle renh, quad_info info);
	tz::error_code quad_renderer_destroy_quad(quad_renderer_handle renh, quad_handle quad);

	void quad_renderer_set_quad_extra_data(quad_renderer_handle renh, quad_handle quad, std::span<const std::byte> data);

	/**
	 * @ingroup tz_ren_quad
	 * @brief Associate an existing image resource with the provided quad renderer, allowing quads to use it as a texture.
	 * @return On success, a integer representing the texture-id corresponding to the image. You can cause a quad to use this texture via @ref set_quad_texture.
	 * @return On failure: Any error returned by the automatic call to @ref pass_add_image_resource.
	 */
	std::expected<std::uint32_t, tz::error_code> quad_renderer_add_texture(quad_renderer_handle renh, tz::gpu::resource_handle image);

	/**
	 * @ingroup tz_ren_quad
	 * @brief Retrieve the position of a quad, in world-space.
	 */
	tz::v2f get_quad_position(quad_renderer_handle renh, quad_handle quad);
	/**
	 * @ingroup tz_ren_quad
	 * @brief Set a new position of a quad, in world-space.
	 */
	void set_quad_position(quad_renderer_handle renh, quad_handle quad, tz::v2f position);

	/**
	 * @ingroup tz_ren_quad
	 * @brief Get the layer value of a given quad.
	 *
	 * Note that layer values only have an effect if layering (see @ref quad_renderer_flag::enable_layering) is enabled.
	 */
	short get_quad_layer(quad_renderer_handle renh, quad_handle quad);
	/**
	 * @ingroup tz_ren_quad
	 * @brief Set the layer value of a given quad.
	 * @param layer New layer value. Should be between -100 and 100.
	 *
	 * Note that layer values only have an effect if layering (see @ref quad_renderer_flag::enable_layering) is enabled.
	 */
	void set_quad_layer(quad_renderer_handle renh, quad_handle quad, short layer);

	float get_quad_rotation(quad_renderer_handle renh, quad_handle quad);
	void set_quad_rotation(quad_renderer_handle renh, quad_handle quad, float rotation);

	/**

	 * @ingroup tz_ren_quad
	 * @brief Retrieve the scale factor of a quad, in both dimensions.
	 */
	tz::v2f get_quad_scale(quad_renderer_handle renh, quad_handle quad);
	/**
	 * @ingroup tz_ren_quad
	 * @brief Set a new scale factor of a quad, in both dimensions.
	 */
	void set_quad_scale(quad_renderer_handle renh, quad_handle quad, tz::v2f scale);

	/**
	 * @ingroup tz_ren_quad
	 * @brief Retrieve the colour of a quad.
	 *
	 * See @ref quad_info::colour for more details.
	 */
	tz::v3f get_quad_colour(quad_renderer_handle renh, quad_handle quad);
	/**
	 * @ingroup tz_ren_quad
	 * @brief Set a new colour of a quad.
	 *
	 * See @ref quad_info::colour for more details.
	 */
	void set_quad_colour(quad_renderer_handle renh, quad_handle quad, tz::v3f colour);

	/**
	 * @ingroup tz_ren_quad
	 * @brief Retrieve the first texture-id currently being used by a quad.
	 * @return (-1) If the quad was not using a texture.
	 */
	std::uint32_t get_quad_texture0(quad_renderer_handle renh, quad_handle quad);
	/**
	 * @ingroup tz_ren_quad
	 * @brief Retrieve the second texture-id currently being used by a quad.
	 * @return (-1) If the quad was not using a texture.
	 */
	std::uint32_t get_quad_texture1(quad_renderer_handle renh, quad_handle quad);
	/**
	 * @ingroup tz_ren_quad
	 * @brief Set a new first texture-id to be used by a quad.
	 *
	 * Note: Passing -1 as the texture-id will cause the quad to no longer sample from a texture.
	 */
	void set_quad_texture0(quad_renderer_handle renh, quad_handle quad, std::uint32_t texture_id);
	/**
	 * @ingroup tz_ren_quad
	 * @brief Set a new second texture-id to be used by a quad.
	 *
	 * Note: Passing -1 as the texture-id will cause the quad to no longer sample from a texture.
	 */
	void set_quad_texture1(quad_renderer_handle renh, quad_handle quad, std::uint32_t texture_id);

	/**
	 * @ingroup tz_ren_quad
	 * @brief Retrieve a graph representing the quad renderer.
	 *
	 * To actually execute a quad renderer, you must retrieve it's graph and pass it to @ref tz::gpu::execute. See @ref tz_gpu_graph for more info.
	 */
	tz::gpu::graph_handle quad_renderer_graph(quad_renderer_handle renh);
	/**
	 * @ingroup tz_ren_quad
	 * @brief Update internal state for a quad renderer.
	 *
	 * - This does *not* render anything.
	 * - You should call this every frame, before rendering.
	 */
	void quad_renderer_update(quad_renderer_handle renh);
}

#endif // TOPAZ_REN_QUAD_HPP
