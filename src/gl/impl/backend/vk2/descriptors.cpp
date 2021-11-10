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

	DescriptorLayoutBuilderBindless::DescriptorLayoutBuilderBindless(const LogicalDevice& logical_device):
	logical_device(&logical_device),
	descriptors()
	{
		tz_assert(this->logical_device->get_features().contains(DeviceFeature::BindlessDescriptors), "Creating a builder for a bindless descriptor layout, but the LogicalDevice does not have DeviceFeature::BindlessDescriptors enabled");
	}

	DescriptorLayoutBuilderBindless& DescriptorLayoutBuilderBindless::with_descriptor(DescriptorType desc, std::size_t descriptor_count)
	{
		this->descriptors.push_back({.type = desc, .count = static_cast<std::uint32_t>(descriptor_count)});
		return *this;
	}

	DescriptorLayoutInfo DescriptorLayoutBuilderBindless::build() const
	{
		constexpr VkDescriptorBindingFlags vk_bindless_flags_head =
			VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
			VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT |
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
		constexpr VkDescriptorBindingFlags vk_bindless_flags_tail =
			vk_bindless_flags_head |
			VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

		DescriptorLayoutBindlessFlagsInfo bindless_flags;
		bindless_flags.binding_flags.resize(this->descriptors.size());
		// binding_flags = {head, head, head, ..., head, head, tail}
		std::for_each(bindless_flags.binding_flags.begin(), bindless_flags.binding_flags.end(),
		[](VkDescriptorBindingFlags& flags){flags = vk_bindless_flags_head;});
		bindless_flags.binding_flags.back() = vk_bindless_flags_tail;

		DescriptorLayoutInfo info;
		info.context = DescriptorContext::Bindless;
		info.maybe_bindless_flags = bindless_flags;
		info.bindings.resize(this->descriptors.size());
		for(std::size_t i = 0; i < this->descriptors.size(); i++)
		{
			const DescriptorLayoutElementInfo& elem = this->descriptors[i];

			VkDescriptorSetLayoutBinding& binding = info.bindings[i];
			binding.binding = i;
			binding.descriptorType = static_cast<VkDescriptorType>(elem.type);
			binding.descriptorCount = elem.count;
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
		VkDescriptorSetLayoutCreateInfo create{};
		VkDescriptorSetLayoutBindingFlagsCreateInfo create_bindless{};
		switch(info.context)
		{
			case DescriptorContext::Classic:
				create =
				{
					.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.bindingCount = static_cast<std::uint32_t>(info.bindings.length()),
					.pBindings = info.bindings.data()
				};
			break;
			case DescriptorContext::Bindless:
				tz_assert(info.maybe_bindless_flags.has_value(), "DescriptorLayoutInfo contains DescriptorContext::Bindless, but no bindless flags were specified. Please submit a bug report");
				create_bindless = info.maybe_bindless_flags.value().native();
				create = 
				{
					.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
					.pNext = &create_bindless,
					.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
					.bindingCount = static_cast<std::uint32_t>(info.bindings.length()),
					.pBindings = info.bindings.data()
				};
			break;
			default:
				tz_error("Unrecognised DescriptorContext");
			break;
		}

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
