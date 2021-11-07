#include "gl/impl/backend/vk2/hardware/queue.hpp"
#if TZ_VULKAN
#include "gl/impl/backend/vk2/descriptors.hpp"

namespace tz::gl::vk2
{
	DescriptorLayoutBindlessFlagsInfo::NativeType DescriptorLayoutBindlessFlagsInfo::native() const
	{
		return
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
			.pNext = nullptr,
			.bindingCount = static_cast<std::uint32_t>(this->binding_flags.length()),
			.pBindingFlags = this->binding_flags.data()
		};
	}

	DescriptorLayoutInfo::NativeType DescriptorLayoutInfo::native() const
	{
		switch(this->context)
		{
			case DescriptorContext::Classic:
				return
				{
					.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.bindingCount = static_cast<std::uint32_t>(this->bindings.length()),
					.pBindings = this->bindings.data()
				};
			break;
			case DescriptorContext::Bindless:
				tz_assert(this->maybe_bindless_flags.has_value(), "DescriptorLayoutInfo contains DescriptorContext::Bindless, but no bindless flags were specified. Please submit a bug report");
				return
				{
					.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
					.pNext = &this->maybe_bindless_flags.value(),
					.bindingCount = static_cast<std::uint32_t>(this->bindings.length()),
					.pBindings = this->bindings.data()
				};
			break;
			default:
				tz_error("Unrecognised DescriptorContext");
			break;
		}
	}

	DescriptorLayoutBuilder::DescriptorLayoutBuilder(const LogicalDevice& logical_device):
	logical_device(&logical_device),
	descriptors()
	{}

	DescriptorLayoutBuilder& DescriptorLayoutBuilder::with_descriptor(DescriptorType desc)
	{
		this->descriptors.push_back(desc);
		return *this;
	}

	DescriptorLayoutInfo DescriptorLayoutBuilder::build() const
	{
		DescriptorLayoutInfo info;
		info.context = DescriptorContext::Classic;
		info.bindings.resize(this->descriptors.size());
		for(std::size_t i = 0; i < this->descriptors.size(); i++)
		{
			VkDescriptorSetLayoutBinding& binding = info.bindings[i];
			binding.binding = i;
			binding.descriptorType = static_cast<VkDescriptorType>(this->descriptors[i]);
			binding.descriptorCount = 1;
			// TODO: Improve
			binding.stageFlags = VK_SHADER_STAGE_ALL;
			// TODO: Improve
			binding.pImmutableSamplers = nullptr;
		}
		info.logical_device = this->logical_device;
		return info;
	}

	DescriptorLayout::DescriptorLayout(const DescriptorLayoutInfo& info):
	descriptor_layout(VK_NULL_HANDLE),
	logical_device(info.logical_device)
	{
		if(info.context == DescriptorContext::Bindless)
		{
			tz_error("DescriptorContext::Bindless not yet implemented");
		}

		VkDescriptorSetLayoutCreateInfo create = info.native();
		VkResult res = vkCreateDescriptorSetLayout(this->logical_device->native(), &create, nullptr, &this->descriptor_layout);
		switch(res)
		{
			case VK_SUCCESS:
				// do nothing
			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				tz_error("Failed to create DescriptorLayout because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz_error("Failed to create DescriptorLayout because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
			break;
			default:
				tz_error("Failed to create DescriptorSetLayout but cannot determine why. Please submit a bug report.");
			break;
		}
	}

	DescriptorLayout::DescriptorLayout(DescriptorLayout&& move):
	descriptor_layout(VK_NULL_HANDLE),
	logical_device(nullptr)
	{
		*this = std::move(move);
	}

	DescriptorLayout::~DescriptorLayout()
	{
		if(this->descriptor_layout != VK_NULL_HANDLE)
		{
			tz_assert(this->logical_device != nullptr && !this->logical_device->is_null(), "Cannot destroy DescriptorLayout because LogicalDevice provided is nullptr or a null device.");
			vkDestroyDescriptorSetLayout(this->logical_device->native(), this->descriptor_layout, nullptr);
			this->descriptor_layout = VK_NULL_HANDLE;
		}
	}

	DescriptorLayout& DescriptorLayout::operator=(DescriptorLayout&& rhs)
	{
		std::swap(this->descriptor_layout, rhs.descriptor_layout);
		std::swap(this->logical_device, rhs.logical_device);
		return *this;
	}

	DescriptorLayout::NativeType DescriptorLayout::native() const
	{
		return this->descriptor_layout;
	}
}

#endif // TZ_VULKAN
