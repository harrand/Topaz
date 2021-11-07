#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_FIXED_FUNCTION_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_FIXED_FUNCTION_HPP
#include "core/containers/basic_list.hpp"
#include "core/vector.hpp"
#include "vulkan/vulkan.h"

namespace tz::gl::vk2
{
	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_fixed
	 * Specifies how the input vertex data is organised.
	 */
	struct VertexInputState
	{
		using Binding = VkVertexInputBindingDescription;
		using Attribute = VkVertexInputAttributeDescription;

		tz::BasicList<Binding> bindings = {};
		tz::BasicList<Attribute> attributes = {};
		
		using NativeType = VkPipelineVertexInputStateCreateInfo;
		NativeType native() const;
	};

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

	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_fixed
	 * When two fragments depth values are compared aka `comp(lhs, rhs)`, what should the return expression be?
	 */
	enum class DepthComparator
	{
		/// - Return false.
		AlwaysFalse = VK_COMPARE_OP_NEVER,
		/// - Return lhs < rhs.
		LessThan = VK_COMPARE_OP_LESS,
		/// - Return lhs == rhs.
		EqualTo = VK_COMPARE_OP_EQUAL,
		/// - Return lhs <= rhs.
		LessThanOrEqual = VK_COMPARE_OP_LESS_OR_EQUAL,
		/// - Return lhs > rhs.
		GreaterThan = VK_COMPARE_OP_GREATER,
		/// - Return lhs != rhs.
		NotEqualTo = VK_COMPARE_OP_NOT_EQUAL,
		/// - Return lhs >= rhs.
		GreaterThanOrEqual = VK_COMPARE_OP_GREATER_OR_EQUAL,
		/// - Return true.
		AlwaysTrue = VK_COMPARE_OP_ALWAYS
	};
	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_fixed
	 * Specifies the state of the depth/stencil buffer, if any.
	 */
	struct DepthStencilState
	{
		/// Whether depth testing is enabled.
		bool depth_testing = false;
		/// If a fragment's value passes the depth test (that is, the depth compare operation returns `true`), this flag controls whether the depth attachment value is set to the sample's depth value.
		bool depth_writes = false;
		/// Describes how two fragment depth values are compared. See @ref DepthComparator for more information.
		DepthComparator depth_compare_operation = DepthComparator::LessThan;
		/// Whether depth bounds testing should be enabled. If enabled, fragments with depth values < `min_depth_bounds` || >`max_depth_bounds` will have zero coverage.
		bool depth_bounds_testing = false;

		/// Whether stencil testing is enabled.
		bool stencil_testing = false;
		/// Parameter of stencil testing.
		VkStencilOpState front = {};
		/// Parameter of stencil testing.
		VkStencilOpState back = {};
		/// If depth bounds testing is enabled, this specifies the minimum depth boundary.
		float min_depth_bounds = 0.0f;
		/// If depth bounds testing is enabled, this specifies the maximum depth boundary.
		float max_depth_bounds = 1.0f;

		using NativeType = VkPipelineDepthStencilStateCreateInfo;
		NativeType native() const;
	};

	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_fixed
	 * Specifies how a new fragment colour is combined with the previous colour within the output.
	 */
	struct ColourBlendState
	{
		using AttachmentState = VkPipelineColorBlendAttachmentState; 
		static AttachmentState no_blending();

		tz::BasicList<AttachmentState> attachment_states = {no_blending()};
		std::optional<VkLogicOp> logical_operator = std::nullopt;
		tz::Vec4 blend_constants{0.0f, 0.0f, 0.0f, 0.0f};
		
		using NativeType = VkPipelineColorBlendStateCreateInfo;
		NativeType native() const;
	};

	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_fixed
	 * At present, dynamic state is not supported, so this struct is not configurable.
	 * TODO: Implement
	 */
	struct DynamicState
	{
		using NativeType = VkPipelineDynamicStateCreateInfo;
		NativeType native() const;
	};
}

#endif // TOPAZ_GL_IMPL_BACKEND_VK2_FIXED_FUNCTION_HPP
