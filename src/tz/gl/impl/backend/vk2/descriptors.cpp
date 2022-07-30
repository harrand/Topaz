#if TZ_VULKAN
#include "tz/core/profiling/zone.hpp"
#include "tz/gl/impl/backend/vk2/sampler.hpp"
#include "tz/gl/impl/backend/vk2/descriptors.hpp"
#include <numeric>

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
		TZ_PROFZONE("Vulkan Backend - Descriptor Layout Create", TZ_PROFCOL_RED);
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
		VkDescriptorSetLayoutCreateFlags layout_flags = 0;
		// If any binding has the VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT bit set, flags must include VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT
		if(std::any_of(binding_flags.begin(), binding_flags.end(),
		[](const VkDescriptorBindingFlags& flags) -> bool
		{
			return flags & VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
		}))
		{
			layout_flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT; 
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
			.flags = layout_flags,
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

	const LogicalDevice& DescriptorLayout::get_device() const
	{
		tz_assert(this->info.logical_device != nullptr, "DescriptorLayoutInfo contained nullptr LogicalDevice. Please submit a bug report.");
		return *this->info.logical_device;
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
		info.logical_device = &layout.get_device();
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

	void DescriptorSet::EditRequest::set_buffer(std::uint32_t binding_id, Write::BufferWriteInfo buffer_write, std::uint32_t array_index)
	{
		// We can find out the DescriptorType at this binding id from the sets layout. Here we make sure it matches.
		#if TZ_DEBUG
			const DescriptorLayoutInfo::BindingInfo& binding_info = this->get_set().get_layout().get_bindings()[binding_id];
			constexpr std::array<DescriptorType, 2> buffer_descriptor_types = {DescriptorType::UniformBuffer, DescriptorType::StorageBuffer};
			tz_assert(std::find(buffer_descriptor_types.begin(), buffer_descriptor_types.end(), binding_info.type) != buffer_descriptor_types.end(), "EditRequest for a DescriptorSet wants to set an image at binding %u, but the DescriptorType at that index according to the layout is not an image.", binding_id);
			if(array_index != 0)
			{
				tz_assert(array_index < binding_info.count, "EditRequest for a DescriptorSet wants to set an image at binding %u and array-index %u, but the array index was out of range. Descriptor array at this binding has length %u", binding_id, array_index, binding_info.count);
			}
		#endif
		
		write_info.add
		({
			.set = this->set,
			.binding_id = binding_id,
			.array_element = array_index,
			.write_infos = {buffer_write}
		});
	}

	void DescriptorSet::EditRequest::set_image(std::uint32_t binding_id, Write::ImageWriteInfo image_write, std::uint32_t array_index)
	{
		// We can find out the DescriptorType at this binding id from the sets layout. Here we make sure it matches.
		#if TZ_DEBUG
			const DescriptorLayoutInfo::BindingInfo& binding_info = this->get_set().get_layout().get_bindings()[binding_id];
			constexpr std::array<DescriptorType, 3> image_descriptor_types = {DescriptorType::Image, DescriptorType::ImageWithSampler, DescriptorType::StorageImage};
			tz_assert(std::find(image_descriptor_types.begin(), image_descriptor_types.end(), binding_info.type) != image_descriptor_types.end(), "EditRequest for a DescriptorSet wants to set an image at binding %u, but the DescriptorType at that index according to the layout is not an image.", binding_id);
			if(array_index != 0)
			{
				tz_assert(array_index < binding_info.count, "EditRequest for a DescriptorSet wants to set an image at binding %u and array-index %u, but the array index was out of range. Descriptor array at this binding has length %u", binding_id, array_index, binding_info.count);
			}
		#endif
		write_info.add
		({
			.set = this->set,
			.binding_id = binding_id,
			.array_element = array_index,
			.write_infos = {image_write}
		});
	}

	DescriptorSet::WriteList DescriptorSet::EditRequest::to_write_list() const
	{
		return this->write_info;
	}

	const DescriptorSet& DescriptorSet::EditRequest::get_set() const
	{
		tz_assert(this->set != nullptr, "DescriptorSet EditRequest has set nullptr, implying it is not correctly referencing an existing DescriptorSet. Please submit a bug report.");
		return *this->set;
	}

	DescriptorSet::EditRequest::EditRequest(DescriptorSet& set):
	set(&set){}

	DescriptorSet::NativeType DescriptorSet::native() const
	{
		return this->set;
	}

	std::size_t DescriptorSet::get_set_id() const
	{
		return this->set_id;
	}

	const DescriptorLayout& DescriptorSet::get_layout() const
	{
		return *this->layout;
	}

	void DescriptorSet::set_layout(DescriptorLayout& layout)
	{
		this->layout = &layout;
	}

	DescriptorSet::EditRequest DescriptorSet::make_edit_request()
	{
		return {*this};
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

	void DescriptorPool::UpdateRequest::add_set_edit(DescriptorSet::EditRequest set_edit)
	{
		tz_assert(this->pool->contains(set_edit.get_set()), "Trying to add set EditRequest to a pool UpdateRequest, but the set edited in the request does not belong to the pool. Please submit a bug report.");
		this->set_edits.push_back(set_edit);
	}

	std::span<const DescriptorSet::EditRequest> DescriptorPool::UpdateRequest::get_set_edits() const
	{
		return this->set_edits;
	}

	DescriptorPool::UpdateRequest::UpdateRequest(DescriptorPool& pool):
	pool(&pool){}

	DescriptorPool::DescriptorPool(DescriptorPoolInfo info):
	pool(VK_NULL_HANDLE),
	info(info),
	allocated_set_natives()
	{
		TZ_PROFZONE("Vulkan Backend - DescriptorPool Create", TZ_PROFCOL_RED);
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

		VkResult res = vkCreateDescriptorPool(this->get_device().native(), &create, nullptr, &this->pool);
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

			vkDestroyDescriptorPool(this->get_device().native(), this->pool, nullptr);
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

	bool DescriptorPool::contains(const DescriptorSet& set) const
	{
		return std::find(this->allocated_set_natives.begin(), this->allocated_set_natives.end(), set.native()) != this->allocated_set_natives.end();
	}

	const LogicalDevice& DescriptorPool::get_device() const
	{
		tz_assert(this->info.logical_device != nullptr, "DescriptorPoolInfo contained nullptr LogicalDevice. Please submit a bug report.");
		return *this->info.logical_device;
	}

	DescriptorPool::AllocationResult DescriptorPool::allocate_sets(const DescriptorPool::Allocation& alloc)
	{
		TZ_PROFZONE("Vulkan Backend - DescriptorPool Set Allocate", TZ_PROFCOL_RED);
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
		VkResult res = vkAllocateDescriptorSets(this->get_device().native(), &create, output_set_natives.data());
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

	DescriptorPool::UpdateRequest DescriptorPool::make_update_request()
	{
		return {*this};
	}

	void DescriptorPool::update_sets(DescriptorPool::UpdateRequest update_request)
	{
		DescriptorSet::WriteList writes;
		for(const DescriptorSet::EditRequest& set_edit : update_request.get_set_edits())
		{
			writes.append(set_edit.to_write_list());
		}
		this->update_sets(writes);
	}

	void DescriptorPool::update_sets(const DescriptorSet::WriteList& writes)
	{
		TZ_PROFZONE("Vulkan Backend - Descriptor Pool Sets Edit", TZ_PROFCOL_RED);
		std::vector<VkWriteDescriptorSet> write_natives(writes.length());

		// Firstly let's find out how many buffers/images we're actually going to need to write to.
		std::size_t num_buffers = std::accumulate(writes.begin(), writes.end(), 0u, [](std::size_t sum, const DescriptorSet::Write& write) -> std::size_t
		{
			return sum + std::count_if(write.write_infos.begin(), write.write_infos.end(), [](const DescriptorSet::Write::WriteInfo& write_info)
			{
				return std::holds_alternative<DescriptorSet::Write::BufferWriteInfo>(write_info);
			});
		});

		std::size_t num_textures = std::accumulate(writes.begin(), writes.end(), 0u, [](std::size_t sum, const DescriptorSet::Write& write) -> std::size_t
		{
			return sum + std::count_if(write.write_infos.begin(), write.write_infos.end(), [](const DescriptorSet::Write::WriteInfo& write_info)
			{
				return std::holds_alternative<DescriptorSet::Write::ImageWriteInfo>(write_info);
			});
		});
		// Now create enough space for them and we'll write into them as we go along.

		std::vector<VkDescriptorBufferInfo> write_buffers(num_buffers);
		std::vector<VkDescriptorImageInfo> write_images(num_textures);

		std::size_t buffer_offset = 0;
		std::size_t image_offset = 0;

		for(std::size_t i = 0; i < writes.length(); i++)
		{
			const DescriptorSet::Write& write = writes[i];
			bool is_buffer = false;
			for(const DescriptorSet::Write::WriteInfo& write_info : write.write_infos)
			{
				std::visit([&](auto&& arg)
				{
					using T = std::decay_t<decltype(arg)>;
					if constexpr(std::is_same_v<T, DescriptorSet::Write::BufferWriteInfo>)
					{
						// We're a buffer.	
						std::size_t buffer_index = buffer_offset++;
						tz_assert(buffer_index < num_buffers, "Buffer Index out of range. Index = %zu, length = %zu. Please submit a bug report", buffer_index, num_buffers);
						write_buffers[buffer_index] = VkDescriptorBufferInfo
						{
							.buffer = arg.buffer->native(),
							.offset = static_cast<VkDeviceSize>(arg.buffer_offset),
							.range = static_cast<VkDeviceSize>(arg.buffer_write_size)
						};
						is_buffer = true;
					}
					else if constexpr(std::is_same_v<T, DescriptorSet::Write::ImageWriteInfo>)
					{
						// We're an image.
						std::size_t image_index = image_offset++;	
						tz_assert(image_index < num_textures, "Image index out of range. Index = %zu, length = %zu. Please submit a bug report", image_index, num_textures);
						const Image& img = arg.image_view->get_image();
						ImageLayout expected_image_layout;
						if(img.get_layout() == ImageLayout::General)
						{
							expected_image_layout = ImageLayout::General;
						}
						else
						{
							expected_image_layout = ImageLayout::ShaderResource;
						}
						write_images[image_index] = VkDescriptorImageInfo
						{
							.sampler = arg.sampler->native(),
							.imageView = arg.image_view->native(),
							.imageLayout = static_cast<VkImageLayout>(expected_image_layout)
						};
						is_buffer = false;
					}
				}, write_info);

			}
			write_natives[i] = VkWriteDescriptorSet
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = nullptr,
				.dstSet = write.set->native(),
				.dstBinding = write.binding_id,
				.dstArrayElement = write.array_element,
				.descriptorCount = static_cast<std::uint32_t>(write.write_infos.size()),
				.descriptorType = detail::to_desc_type(write.set->get_layout().get_bindings()[write.binding_id].type),
				.pImageInfo = nullptr,
				.pBufferInfo = nullptr,
				.pTexelBufferView = nullptr
			};
			if(is_buffer)
			{
				std::size_t idx = buffer_offset - 1;
				tz_assert(idx < num_buffers, "Buffer index out of range. Index = %zu, size = %zu", idx, num_buffers);
				write_natives[i].pBufferInfo = &write_buffers[idx];
			}
			else
			{
				std::size_t idx = image_offset - 1;
				tz_assert(idx < num_textures, "Image index out of range. Index = %zu, size = %zu", idx, num_textures);
				write_natives[i].pImageInfo = &write_images[idx];
			}
		}
		vkUpdateDescriptorSets(this->get_device().native(), write_natives.size(), write_natives.data(), 0, nullptr);
	}

	void DescriptorPool::clear()
	{
		vkResetDescriptorPool(this->get_device().native(), this->pool, 0);
		// Note: vkFreeDescriptorSets can't be used unless VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT is set (which is currently isn't)
		//if(this->allocated_set_natives.empty())
		//{
		//	return;
		//}
		//vkFreeDescriptorSets(this->get_device().native(), this->pool, this->allocated_set_natives.size(), this->allocated_set_natives.data());
		//this->allocated_set_natives.clear();
	}

	DescriptorPool DescriptorPool::null()
	{
		return {};
	}

	bool DescriptorPool::is_null() const
	{
		return this->pool == VK_NULL_HANDLE;
	}

	DescriptorPool::DescriptorPool():
	pool(VK_NULL_HANDLE),
	info(),
	allocated_set_natives(){}
}

#endif // TZ_VULKAN
