#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_PIPELINE_LAYOUT_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_PIPELINE_LAYOUT_HPP
#if TZ_VULKAN
#include "tz/gl/impl/vulkan/detail/descriptors.hpp"

namespace tz::gl::vk2
{
	/**
	 * @ingroup tz_gl_vk_graphics_pipeline
	 * Specifies creation flags for a @ref PipelineLayout.
	 */
	struct PipelineLayoutInfo
	{
		/// List of layouts, in order.
		std::vector<const DescriptorLayout*> descriptor_layouts;
		/// Owning LogicalDevice. Must not be nullptr or null device.
		const LogicalDevice* logical_device;
	};

	/**
	 * @ingroup tz_gl_vk_graphics_pipeline
	 * Represents an interface between shader stages and shader resources in terms of the layout of a group of @ref DescriptorSet.
	 */
	class PipelineLayout
	{
	public:
		PipelineLayout(const PipelineLayoutInfo& info);
		PipelineLayout(const PipelineLayout& copy) = delete;
		PipelineLayout(PipelineLayout&& move);
		~PipelineLayout();

		PipelineLayout& operator=(const PipelineLayout& rhs) = delete;
		PipelineLayout& operator=(PipelineLayout&& rhs);

		const LogicalDevice& get_device() const;

		using NativeType = VkPipelineLayout;
		NativeType native() const;

		static PipelineLayout null();
		bool is_null() const;
	private:
		PipelineLayout();

		VkPipelineLayout pipeline_layout;
		const LogicalDevice* logical_device;
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_PIPELINE_LAYOUT_HPP
