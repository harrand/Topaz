#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_FIXED_FUNCTION_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_FIXED_FUNCTION_HPP
#include "core/containers/basic_list.hpp"
#include "core/vector.hpp"
#include "vulkan/vulkan.h"

namespace tz::gl::vk2
{
	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_fixed
	 * Specifies what type of geometry shall be drawn.
	 * @note At present, this is not configurable and triangle lists are always drawn.
	 */
	struct InputAssembly
	{
		using NativeType = VkPipelineInputAssemblyStateCreateInfo;
		NativeType native() const;
	};

	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_fixed
	 * Specifies the region of the output that will be rendered to.
	 *
	 * See @ref create_basic_viewport(tz::Vec2) to create a ViewportState for a basic region.
	 */
	struct ViewportState
	{
		tz::BasicList<VkViewport> viewports;
		tz::BasicList<VkRect2D> scissors;

		using NativeType = VkPipelineViewportStateCreateInfo;
		NativeType native() const;
	};

	ViewportState create_basic_viewport(tz::Vec2 dimensions);

	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_fixed
	 * Describes how fragments are generated for geometry.
	 */
	enum class PolygonMode
	{
		/// - Fill the polygon with fragments.
		Fill = VK_POLYGON_MODE_FILL,
		/// - Draw only the polygon edges via lines (wireframe).
		Line = VK_POLYGON_MODE_LINE,
		/// - Draw the polygon vertices as points.
		Point = VK_POLYGON_MODE_POINT
	};

	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_fixed
	 * Describes which faces should be culled.
	 */
	enum class CullMode
	{
		/// - Don't cull anything.
		NoCulling,
		/// - Cull the front face.
		FrontCulling,
		/// - Cull the back face.
		BackCulling,
		/// - Cull everything.
		BothCulling
	};

	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_fixed
	 * Configures how the geometry shaped by input data is transformed into fragments.
	 */
	struct RasteriserState
	{
		/// Fragments that are beyond the near and far planes are clipped to them instead of being discarded. This may be useful for shadow mapping. @note This must be supported by the relevant @ref PhysicalDevice.
		bool depth_clamp = false;
		/// Describe how fragments are generated from geometry. See @ref PolygonMode for more information.
		PolygonMode polygon_mode = PolygonMode::Fill;
		/// Describe the culling behaviour. See @ref CullMode for more information.
		CullMode cull_mode = CullMode::NoCulling;

		using NativeType = VkPipelineRasterizationStateCreateInfo;
		NativeType native() const;
	};

	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_fixed
	 * At present, multisampling is not supported, so this struct is not configurable.
	 * TODO: Implement
	 */
	struct MultisampleState
	{
		using NativeType = VkPipelineMultisampleStateCreateInfo;
		NativeType native() const;
	};
}

#endif // TOPAZ_GL_IMPL_BACKEND_VK2_FIXED_FUNCTION_HPP
