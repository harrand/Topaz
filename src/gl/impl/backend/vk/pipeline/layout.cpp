#if TZ_VULKAN
#include "gl/impl/backend/vk/pipeline/layout.hpp"
#include "core/assert.hpp"

namespace tz::gl::vk::pipeline
{
	Layout::Layout(const LogicalDevice& device):
	Layout(device, DescriptorSetLayoutRefs{}){}

	Layout::Layout(const LogicalDevice& device, DescriptorSetLayoutRefs descriptor_layouts):
	layout(VK_NULL_HANDLE),
	descriptor_layouts(descriptor_layouts),
	device(&device)
	{
		std::vector<VkDescriptorSetLayout> layout_natives;
		for(const auto& descriptor : this->descriptor_layouts)
		{
			layout_natives.push_back(descriptor.get().native());
		}
		VkPipelineLayoutCreateInfo create{};
		create.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		create.setLayoutCount = this->descriptor_layouts.size();
		create.pSetLayouts = layout_natives.data();
		create.pushConstantRangeCount = 0;
		create.pPushConstantRanges = nullptr;

		auto res = vkCreatePipelineLayout(this->device->native(), &create, nullptr, &this->layout);
		tz_assert(res == VK_SUCCESS, "tz::gl::vk::pipeline::Layout::Layout(...): Failed to create pipeline layout");
	}

	Layout::Layout(Layout&& move):
	layout(VK_NULL_HANDLE),
	device(nullptr)
	{
		*this = std::move(move);
	}

	Layout::~Layout()
	{
		if(this->layout != VK_NULL_HANDLE)
		{
			vkDestroyPipelineLayout(this->device->native(), this->layout, nullptr);
			this->layout = VK_NULL_HANDLE;
		}
	}

	Layout& Layout::operator=(Layout&& rhs)
	{
		std::swap(this->layout, rhs.layout);
		std::swap(this->descriptor_layouts, rhs.descriptor_layouts);
		std::swap(this->device, rhs.device);
		return *this;
	}

	VkPipelineLayout Layout::native() const
	{
		return this->layout;
	}
}

#endif // TZ_VULKAN