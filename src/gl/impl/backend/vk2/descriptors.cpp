#if TZ_VULKAN
#include "gl/impl/backend/vk2/descriptors.hpp"

namespace tz::gl::vk2
{
	namespace detail
	{
		VkDescriptorType to_desc_type(DescriptorType type)
		{
			return vk_desc_types[static_cast<int>(type)];
		}
	}

	std::size_t DescriptorLayoutInfo::binding_count() const
	{
		return this->bindings.length();
	}

	bool DescriptorLayoutInfo::has_valid_device() const
	{
		return this->logical_device != nullptr && !this->logical_device->is_null();
	}

	bool DescriptorLayoutInfo::device_supports_flags() const
	{
		if(!this->has_valid_device())
		{
			return false;
		}

		for(const BindingInfo& binding : this->bindings)
		{
			if(binding.flags.empty())
			{
				continue;
			}
			for(DescriptorFlag flag : binding.flags)
			{
				switch(flag)
				{
					case DescriptorFlag::UpdateAfterBind:
					[[fallthrough]];
					case DescriptorFlag::UpdateUnusedWhilePending:
					[[fallthrough]];
					case DescriptorFlag::PartiallyBound:
					[[fallthrough]];
					case DescriptorFlag::VariableCount:
						if(!this->logical_device->get_features().contains(DeviceFeature::BindlessDescriptors))
						{
							return false;
						}
					break;
				}
			}
		}
		return true;
	}

	DescriptorLayoutBuilder::DescriptorLayoutBuilder(DescriptorLayoutInfo existing_info):
	info(existing_info){}

	DescriptorLayoutBuilder& DescriptorLayoutBuilder::with_binding(DescriptorLayoutInfo::BindingInfo binding)
	{
		this->info.bindings.add(binding);
		return *this;
	}

	const LogicalDevice* DescriptorLayoutBuilder::get_device() const
	{
		return this->info.logical_device;
	}

	void DescriptorLayoutBuilder::set_device(const LogicalDevice& device)
	{
		this->info.logical_device = &device;
	}

	const DescriptorLayoutInfo& DescriptorLayoutBuilder::get_info() const
	{
		return this->info;
	}

	DescriptorLayout DescriptorLayoutBuilder::build() const
	{
		return {this->get_info()};
	}

	void DescriptorLayoutBuilder::clear()
	{
		this->info = {};
	}

	DescriptorLayout::DescriptorLayout(DescriptorLayoutInfo info):
	descriptor_layout(VK_NULL_HANDLE),
	info(info)
	{
		// We need a valid LogicalDevice. The info should have that.
		tz_assert(info.has_valid_device(), "DescriptorLayoutInfo did not have valid LogicalDevice. Please submit a bug report");
		tz_assert(info.device_supports_flags(), "DescriptorLayoutInfo contained bindings with flags which are not supported by the LogicalDevice. Please submit a bug report.");

		// Convert the binding information from the info struct to something VKAPI-friendly.
		std::vector<VkDescriptorBindingFlags> binding_flags(info.binding_count());
		std::vector<VkDescriptorSetLayoutBinding> bindings(info.binding_count());

		for(std::uint32_t i = 0; std::cmp_less(i, info.bindings.length()); i++)
		{
			const DescriptorLayoutInfo::BindingInfo& binding = info.bindings[i];
			binding_flags[i] = static_cast<VkDescriptorBindingFlags>(static_cast<DescriptorFlag>(binding.flags));
			bindings[i] =
			{
				.binding = i,
				.descriptorType = detail::to_desc_type(binding.type),
				.descriptorCount = binding.count,
				.stageFlags = VK_SHADER_STAGE_ALL,
				.pImmutableSamplers = nullptr
			};
		}
		VkDescriptorSetLayoutCreateFlags flags = 0;
		// If any binding has the VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT bit set, flags must include VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT
		if(std::any_of(binding_flags.begin(), binding_flags.end(),
		[](const VkDescriptorBindingFlags& flags) -> bool
		{
			return flags & VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
		}))
		{
			flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT; 
		}

		// Then fill the VKAPI createinfos.

		VkDescriptorSetLayoutBindingFlagsCreateInfo binding_create
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
			.pNext = nullptr,
			.bindingCount = static_cast<std::uint32_t>(info.binding_count()),
			.pBindingFlags = binding_flags.data()
		};

		VkDescriptorSetLayoutCreateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = &binding_create,
			.flags = flags,
			.bindingCount = static_cast<std::uint32_t>(info.binding_count()),
			.pBindings = bindings.data()
		};

		// Then finally create the layout.
		VkResult res = vkCreateDescriptorSetLayout(info.logical_device->native(), &create, nullptr, &this->descriptor_layout);
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
	DescriptorLayout()
	{
		*this = std::move(move);
	}

	DescriptorLayout::~DescriptorLayout()
	{
		if(this->descriptor_layout != VK_NULL_HANDLE)
		{
			tz_assert(info.has_valid_device(), "DescriptorLayoutInfo did not have valid LogicalDevice. Please submit a bug report");
			vkDestroyDescriptorSetLayout(this->info.logical_device->native(), this->descriptor_layout, nullptr);
			this->descriptor_layout = VK_NULL_HANDLE;
		}
	}

	DescriptorLayout& DescriptorLayout::operator=(DescriptorLayout&& rhs)
	{
		std::swap(this->descriptor_layout, rhs.descriptor_layout);
		std::swap(this->info, rhs.info);
		return *this;
	}

	std::size_t DescriptorLayout::binding_count() const
	{
		return this->info.binding_count();
	}

	std::size_t DescriptorLayout::descriptor_count() const
	{
		std::size_t total_count = 0;
		for(const DescriptorLayoutInfo::BindingInfo& binding : this->info.bindings)
		{
			total_count += binding.count;
		}
		return total_count;
	}

	std::size_t DescriptorLayout::descriptor_count_of(DescriptorType type) const
	{

		std::size_t total_count = 0;
		for(const DescriptorLayoutInfo::BindingInfo& binding : this->info.bindings)
		{
			if(binding.type == type)
			{
				total_count += binding.count;
			}
		}
		return total_count;
	}

	std::span<const DescriptorLayoutInfo::BindingInfo> DescriptorLayout::get_bindings() const
	{
		return this->info.bindings;
	}

	const LogicalDevice* DescriptorLayout::get_device() const
	{
		return this->info.logical_device;
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
	info()
	{}

	const DescriptorLayoutInfo& DescriptorLayout::get_info() const
	{
		return this->info;
	}

	DescriptorPoolInfo DescriptorPoolInfo::to_fit_layout(const DescriptorLayout& layout, std::size_t quantity)
	{
		DescriptorPoolInfo info;
		info.logical_device = layout.get_device();
		info.limits.max_sets = quantity;
		for(std::size_t i = 0; std::cmp_less(i, static_cast<int>(DescriptorType::Count)); i++)
		{
			DescriptorType t = static_cast<DescriptorType>(i);
			info.limits.limits[t] = layout.descriptor_count_of(t);
		}

		std::span<const DescriptorLayoutInfo::BindingInfo> binding_span = layout.get_bindings();
		info.limits.supports_update_after_bind = std::any_of(binding_span.begin(), binding_span.end(), [](const DescriptorLayoutInfo::BindingInfo& binding)->bool
		{
			return binding.flags.contains(DescriptorFlag::UpdateAfterBind);
		});
		return info;
	}
	
	bool DescriptorPoolInfo::has_valid_device() const
	{
		return this->logical_device != nullptr && !this->logical_device->is_null();
	}

	DescriptorSet::NativeType DescriptorSet::native() const
	{
		return this->set;
	}

	const DescriptorLayout& DescriptorSet::get_layout() const
	{
		return *this->layout;
	}

	DescriptorSet::DescriptorSet(std::size_t set_id, const DescriptorLayout& layout, NativeType native):
	set(native),
	set_id(set_id),
	layout(&layout)
	{}

	bool DescriptorPool::AllocationResult::success() const
	{
		return this->type == DescriptorPool::AllocationResult::AllocationResultType::Success;
	}

	DescriptorPool::DescriptorPool(DescriptorPoolInfo info):
	pool(VK_NULL_HANDLE),
	info(info),
	allocated_set_natives()
	{
		tz_assert(this->info.has_valid_device(), "DescriptorPoolInfo did not have valid LogicalDevice. Please submit a bug report.");

		VkDescriptorPoolCreateFlags flags = 0;
		if(this->info.limits.supports_update_after_bind)
		{
			flags |= VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
		}
		std::vector<VkDescriptorPoolSize> pool_sizes;
		for(const auto& [descriptor_type, count] : this->info.limits.limits)
		{
			if(count > 0)
			{
				pool_sizes.push_back(VkDescriptorPoolSize
				{
					.type = detail::to_desc_type(descriptor_type),
					.descriptorCount = count
				});
			}
		}

		VkDescriptorPoolCreateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = flags,
			.maxSets = this->info.limits.max_sets,
			.poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size()),
			.pPoolSizes = pool_sizes.data()
		};

		VkResult res = vkCreateDescriptorPool(this->get_device()->native(), &create, nullptr, &this->pool);
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
	info(),
	allocated_set_natives()
	{
		*this = std::move(move);
	}

	DescriptorPool::~DescriptorPool()
	{
		if(this->pool != VK_NULL_HANDLE)
		{
			this->clear();

			vkDestroyDescriptorPool(this->get_device()->native(), this->pool, nullptr);
			this->pool = VK_NULL_HANDLE;
		}
	}

	DescriptorPool& DescriptorPool::operator=(DescriptorPool&& rhs)
	{
		std::swap(this->pool, rhs.pool);
		std::swap(this->info, rhs.info);
		std::swap(this->allocated_set_natives, rhs.allocated_set_natives);
		return *this;
	}

	const LogicalDevice* DescriptorPool::get_device() const
	{
		return this->info.logical_device;
	}

	DescriptorPool::AllocationResult DescriptorPool::allocate_sets(const DescriptorPool::Allocation& alloc)
	{
		// One or more of our set layouts might contain a binding that is variable-size.
		// Initially we will set the variable count to the max (which is stored in the set layout). Otherwise we will set it to zero.
		std::vector<std::uint32_t> variable_counts(alloc.set_layouts.length());
		std::transform(alloc.set_layouts.begin(), alloc.set_layouts.end(), variable_counts.begin(),
		[](const DescriptorLayout* layout) -> std::uint32_t
		{
			if(layout == nullptr)
			{
				return 0;
			}
			// If the layout does have a variable count, the last one must have it.
			const DescriptorLayoutInfo::BindingInfo& last_binding = layout->get_bindings().back();
			if(last_binding.flags.contains(DescriptorFlag::VariableCount))
			{
				return last_binding.count;
			}
			return 0;
		});
		// Then we retrieve the natives of the layouts.
		std::vector<DescriptorLayout::NativeType> layout_natives(alloc.set_layouts.length());
		std::transform(alloc.set_layouts.begin(), alloc.set_layouts.end(), layout_natives.begin(),
		[](const DescriptorLayout* layout) -> DescriptorLayout::NativeType
		{
			tz_assert(layout != nullptr, "DescriptorPool::Allocate::set_layouts contained nullptr");
			return layout->native();
		});

		VkDescriptorSetVariableDescriptorCountAllocateInfo variable_alloc
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorSetCount = static_cast<std::uint32_t>(variable_counts.size()),
			.pDescriptorCounts = variable_counts.data()
		};

		VkDescriptorSetAllocateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = &variable_alloc,
			.descriptorPool = this->pool,
			.descriptorSetCount = static_cast<std::uint32_t>(alloc.set_layouts.length()),
			.pSetLayouts = layout_natives.data()
		};

		std::vector<DescriptorSet::NativeType> output_set_natives(alloc.set_layouts.length());
		VkResult res = vkAllocateDescriptorSets(this->get_device()->native(), &create, output_set_natives.data());
		DescriptorPool::AllocationResult ret;
		switch(res)
		{
			case VK_SUCCESS:
				ret.type = AllocationResult::AllocationResultType::Success;
			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				tz_error("Failed to allocate DescriptorSets because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
				ret.type = AllocationResult::AllocationResultType::FatalError;
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz_error("Failed to allocate DescriptorSets because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
				ret.type = AllocationResult::AllocationResultType::FatalError;
			break;
			case VK_ERROR_FRAGMENTED_POOL:
				ret.type = AllocationResult::AllocationResultType::FragmentedPool;
			break;
			case VK_ERROR_OUT_OF_POOL_MEMORY:
				ret.type = AllocationResult::AllocationResultType::PoolOutOfMemory;
			break;
			default:
				tz_error("Failed to allocate DescriptorSets but cannot determine why. Please submit a bug report.");
			break;
		}

		// Now we have all the natives, emplace them all within a list of DescriptorSet and then return it.
		for(std::size_t i = 0; i < output_set_natives.size(); i++)
		{
			ret.sets.add(DescriptorSet{i, *alloc.set_layouts[i], output_set_natives[i]});
			this->allocated_set_natives.push_back(output_set_natives[i]);
		}
		return ret;
	}

	void DescriptorPool::update_sets(const DescriptorSet::WriteList& writes)
	{
		tz_error("Not yet implemented");
		std::vector<VkWriteDescriptorSet> write_natives(writes.length());
		std::transform(writes.begin(), writes.end(), write_natives.begin(),
		[](const DescriptorSet::Write& write) -> VkWriteDescriptorSet
		{
			return
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = nullptr,
				.dstSet = write.set->native(),
				.dstBinding = write.binding_id,
				.dstArrayElement = write.array_element,
				.descriptorCount = static_cast<std::uint32_t>(write.write_infos.size()),
				.descriptorType = detail::to_desc_type(write.set->get_layout().get_bindings()[write.binding_id].type),
				// TODO: Fill in if we're writing images
				.pImageInfo = nullptr,
				// TODO: Fill in if we're writing buffers
				.pBufferInfo = nullptr,
				.pTexelBufferView = nullptr
			};
		});
		vkUpdateDescriptorSets(this->get_device()->native(), write_natives.size(), write_natives.data(), 0, nullptr);
	}

	void DescriptorPool::clear()
	{
		vkResetDescriptorPool(this->get_device()->native(), this->pool, 0);
		// Note: vkFreeDescriptorSets can't be used unless VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT is set (which is currently isn't)
		//if(this->allocated_set_natives.empty())
		//{
		//	return;
		//}
		//vkFreeDescriptorSets(this->get_device()->native(), this->pool, this->allocated_set_natives.size(), this->allocated_set_natives.data());
		//this->allocated_set_natives.clear();
	}
}

#endif // TZ_VULKAN
