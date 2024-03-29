#if TZ_VULKAN
#include "tz/core/profile.hpp"
#include "tz/gl/impl/vulkan/detail/pipeline_layout.hpp"

namespace tz::gl::vk2
{
	PipelineLayout::PipelineLayout(const PipelineLayoutInfo& info):
	pipeline_layout(VK_NULL_HANDLE),
	logical_device(info.logical_device)
	{
		TZ_PROFZONE("Vulkan Backend - PipelineLayout Create", 0xFFAA0000);
		tz::assert(this->logical_device != nullptr && !this->logical_device->is_null(), "PipelineLayoutInfo contained nullptr or null LogicalDevice. Please submit a bug report.");
		VkPipelineLayoutCreateInfo create{};
		create.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO; 
		create.pNext = nullptr;
		create.flags = 0;
		create.setLayoutCount = static_cast<std::uint32_t>(info.descriptor_layouts.size());
		std::vector<DescriptorLayout::NativeType> descriptor_layout_natives;
		for(std::size_t i = 0; i < info.descriptor_layouts.size(); i++)
		{
			descriptor_layout_natives.push_back(info.descriptor_layouts[i]->native());
		}
		create.pSetLayouts = descriptor_layout_natives.data();
		create.pushConstantRangeCount = 0;
		create.pPushConstantRanges = nullptr;
		VkResult res = vkCreatePipelineLayout(this->logical_device->native(), &create, nullptr, &this->pipeline_layout);
		switch(res)
		{
			case VK_SUCCESS:
				// do nothing
			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				tz::error("Failed to create PipelineLayout because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz::error("Failed to create PipelineLayout because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
			break;
			default:
				tz::error("Failed to create PipelineLayout but cannot determine why. Please submit a bug report.");
			break;
		}
	}

	PipelineLayout::PipelineLayout(PipelineLayout&& move):
	pipeline_layout(VK_NULL_HANDLE),
	logical_device(nullptr)
	{
		*this = std::move(move);
	}

	PipelineLayout::~PipelineLayout()
	{
		if(this->pipeline_layout != VK_NULL_HANDLE)
		{
			tz::assert(this->logical_device != nullptr && !this->logical_device->is_null(), "Failed to destroy PipelineLayout because LogicalDevice was nullptr or null");
			vkDestroyPipelineLayout(this->logical_device->native(), this->pipeline_layout, nullptr);
			this->pipeline_layout = VK_NULL_HANDLE;
		}
	}

	PipelineLayout& PipelineLayout::operator=(PipelineLayout&& rhs)
	{
		std::swap(this->pipeline_layout, rhs.pipeline_layout);
		std::swap(this->logical_device, rhs.logical_device);
		return *this;
	}

	const LogicalDevice& PipelineLayout::get_device() const
	{
		return *this->logical_device;
	}

	PipelineLayout::NativeType PipelineLayout::native() const
	{
		return this->pipeline_layout;
	}

	PipelineLayout PipelineLayout::null()
	{
		return {};
	}

	bool PipelineLayout::is_null() const
	{
		return this->pipeline_layout == VK_NULL_HANDLE;
	}

	PipelineLayout::PipelineLayout():
	pipeline_layout(VK_NULL_HANDLE),
	logical_device(nullptr)
	{

	}
}
#endif // TZ_VULKAN
