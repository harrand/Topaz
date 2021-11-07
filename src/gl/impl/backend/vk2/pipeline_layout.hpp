#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_PIPELINE_LAYOUT_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_PIPELINE_LAYOUT_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk2/descriptors.hpp"

namespace tz::gl::vk2
{
	struct PipelineLayoutInfo
	{
		std::span<const DescriptorLayout> descriptor_layouts;
		const LogicalDevice* logical_device;
	};

	class PipelineLayout
	{
	public:
		PipelineLayout(const PipelineLayoutInfo& info);
		PipelineLayout(const PipelineLayout& copy) = delete;
		PipelineLayout(PipelineLayout&& move);
		~PipelineLayout();

		PipelineLayout& operator=(const PipelineLayout& rhs) = delete;
		PipelineLayout& operator=(PipelineLayout&& rhs);

		using NativeType = VkPipelineLayout;
		NativeType native() const;
	private:
		VkPipelineLayout pipeline_layout;
		const LogicalDevice* logical_device;
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_PIPELINE_LAYOUT_HPP
