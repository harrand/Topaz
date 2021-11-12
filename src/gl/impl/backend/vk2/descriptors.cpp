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

	DescriptorLayout DescriptorLayout::null()
	{
		return {};
	}

	bool DescriptorLayout::is_null() const
	{
		return this->descriptor_layout == VK_NULL_HANDLE;
	}

	DescriptorLayout::DescriptorLayout():
	descriptor_layout(VK_NULL_HANDLE),
	logical_device(nullptr)
	{}

	DescriptorSet::DescriptorSet(std::size_t set_id, const DescriptorLayout& layout, NativeType native):
	set(native),
	set_id(set_id),
	layout(&layout)
	{}

	DescriptorPoolInfo create_pool_for_layout(const DescriptorLayoutInfo& layout_info, std::size_t number_of_sets)
	{
		DescriptorPoolInfo info;
		info.context = layout_info.context;
		info.logical_device = layout_info.logical_device;
		info.maximum_descriptor_set_count = number_of_sets;

		info.pool_sizes.resize(layout_info.bindings.length());
		std::transform(layout_info.bindings.begin(), layout_info.bindings.end(), info.pool_sizes.begin(),
		[number_of_sets](const VkDescriptorSetLayoutBinding& binding)->DescriptorPoolInfo::PoolSize
		{
			return
			{
				.type = binding.descriptorType,
				.descriptorCount = static_cast<std::uint32_t>(binding.descriptorCount * number_of_sets)
			};
		});
		return info;
	}

	DescriptorPool::DescriptorPool(const DescriptorPoolInfo& info):
	pool(VK_NULL_HANDLE),
	context(info.context),
	logical_device(info.logical_device)
	{
		VkDescriptorPoolCreateInfo create{};
		create.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		create.pNext = nullptr;
		create.flags = 0;
		create.maxSets = static_cast<std::uint32_t>(info.maximum_descriptor_set_count);
		create.poolSizeCount = static_cast<std::uint32_t>(info.pool_sizes.length());
		create.pPoolSizes = info.pool_sizes.data();

		if(this->context == DescriptorContext::Bindless)
		{
			create.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
		}

		VkResult res = vkCreateDescriptorPool(this->logical_device->native(), &create, nullptr, &this->pool);
		switch(res)
		{
			case VK_SUCCESS:

			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				tz_error("Failed to create DescriptorPool because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz_error("Failed to create DescriptorPool because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_FRAGMENTATION:
				tz_error("Failed to create DescriptorPool due to fragmentation. There may be a code path to avoid this issue from being lethal, but it does not exist yet. Please submit a bug report.");
			break;
			default:
				tz_error("Failed to create DescriptorPool but cannot determine why. Please submit a bug report.");
			break;
		}
	}

	DescriptorPool::DescriptorPool(DescriptorPool&& move):
	pool(VK_NULL_HANDLE),
	context(DescriptorContext::Classic),
	logical_device(nullptr)
	{
		*this = std::move(move);
	}

	DescriptorPool::~DescriptorPool()
	{
		if(this->pool != VK_NULL_HANDLE)
		{
			this->clear();

			tz_assert(this->logical_device != nullptr && !this->logical_device->is_null(), "Failed to destroy DescriptorPool because LogicalDevice was null or a null device");	
			vkDestroyDescriptorPool(this->logical_device->native(), this->pool, nullptr);
			this->pool = VK_NULL_HANDLE;
		}
	}

	DescriptorPool& DescriptorPool::operator=(DescriptorPool&& rhs)
	{
		std::swap(this->pool, rhs.pool);
		std::swap(this->context, rhs.context);
		std::swap(this->logical_device, rhs.logical_device);
		std::swap(this->allocated_set_natives, rhs.allocated_set_natives);
		return *this;
	}

	tz::BasicList<DescriptorSet> DescriptorPool::allocate_sets(const DescriptorPool::AllocateInfo& alloc)
	{
		// Extension structure which will only be used if we're bindless (need the lifetime to match though so we're chucking it here.
		VkDescriptorSetVariableDescriptorCountAllocateInfo variable_info{}; 

		VkDescriptorSetAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.pNext = nullptr;
		alloc_info.descriptorPool = this->pool;
		alloc_info.descriptorSetCount = alloc.set_layouts.length();

		if(this->context == DescriptorContext::Bindless)
		{
			variable_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
			variable_info.pNext = nullptr;
			// TODO: Make this not zero. Right now we can't have any partially bound variable descriptors in any sets.
			variable_info.descriptorSetCount = 0;
			variable_info.pDescriptorCounts = nullptr;

			alloc_info.pNext = &variable_info;
		}
		
		std::vector<DescriptorLayout::NativeType> layout_natives;
		layout_natives.resize(alloc.set_layouts.length());
		std::transform(alloc.set_layouts.begin(), alloc.set_layouts.end(), layout_natives.begin(), [](const DescriptorLayout* layout){return layout->native();});

		alloc_info.pSetLayouts = layout_natives.data();

		std::vector<DescriptorSet::NativeType> set_natives;
		set_natives.resize(layout_natives.size());

		VkResult res = vkAllocateDescriptorSets(this->logical_device->native(), &alloc_info, set_natives.data());
		switch(res)
		{
			case VK_SUCCESS:

			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				tz_error("Failed to allocate DescriptorSets because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz_error("Failed to allocate DescriptorSets because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_FRAGMENTED_POOL:
				tz_error("There was technically enough memory available to allocate DescriptorSets, but the pool was too fragmented. Logic exists to fix this issue, but it is not yet implemented. Please submit a bug report.");
			break;
			case VK_ERROR_OUT_OF_POOL_MEMORY:
				tz_error("Failed to allocate DescriptorSets because the pool ran out of space. Logic exists to fix this issue, but it is not yet implemented. Please submit a bug report");
			break;
			default:
				tz_error("Failed to allocate DescriptorSets but cannot determine why. Please submit a bug report.");
			break;
		}
		// set_natives contains all the set natives from this specific allocation. there might already be some from previous allocations within this->allocated_set_natives. We should append the new data to that.
		std::copy(this->allocated_set_natives.begin(), this->allocated_set_natives.end(), set_natives.begin());
		// Now we construct the DescriptorSets from the new set natives and return that.
		tz::BasicList<DescriptorSet> ret;
		for(std::size_t i = 0; std::cmp_less(i, set_natives.size()); i++)
		{
			ret.add(DescriptorSet{i, *alloc.set_layouts[i], set_natives[i]});
		}
		return ret;
	}

	void DescriptorPool::clear()
	{
		if(this->allocated_set_natives.empty())
		{
			return;
		}
		VkResult res = vkFreeDescriptorSets(this->logical_device->native(), this->pool, this->allocated_set_natives.size(), this->allocated_set_natives.data());
		switch(res)
		{
			case VK_SUCCESS:

			break;
			default:
				tz_error("Failed to free DescriptorSets but cannot determine why. Please submit a bug report.");
			break;
		}
		this->allocated_set_natives.clear();
	}

	DescriptorPool::NativeType DescriptorPool::native() const
	{
		return this->pool;
	}
}

#endif // TZ_VULKAN
