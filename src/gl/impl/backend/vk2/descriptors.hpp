#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_DESCRIPTORS2_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_DESCRIPTORS2_HPP
#include "core/containers/basic_list.hpp"
#include "gl/impl/backend/vk2/buffer.hpp"
#include "gl/impl/backend/vk2/sampler.hpp"
#include "gl/impl/backend/vk2/image_view.hpp"
#include <unordered_map>

namespace tz::gl::vk2
{
	/**
	 * @ingroup tz_gl_vk_descriptors
	 */
	enum class DescriptorType
	{
		Sampler,
		ImageWithSampler,
		Image,
		StorageImage,
		UniformBuffer,
		StorageBuffer, 

		Count
	};

	namespace detail
	{
		constexpr std::array<VkDescriptorType, static_cast<int>(DescriptorType::Count)> vk_desc_types{VK_DESCRIPTOR_TYPE_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
		VkDescriptorType to_desc_type(DescriptorType type);
	}

	/**
	 * @ingroup tz_gl_vk_descriptors
	 * Information about a specific descriptor (or array of descriptors) via a descriptor layout binding.
	 */
	enum class DescriptorFlag
	{
		/// - Indicates that the parent set can be updated even after its bound within a command buffer.
		UpdateAfterBind = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
		/// - Indicates that the parent set can be updated even if its bound and being used by a command buffer, so long as the execution of that command buffer isn't actually using this descriptor.
		UpdateUnusedWhilePending = VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT,
		/// - Indicates that the descriptor is partially bound. This means that if it's not used during rendering (even if its set is), it doesn't have to be set properly, or even in a valid state.
		PartiallyBound = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
		/// - Indicates that this descriptor array is of variable size. Note that only the final binding of a descriptor layout can be an array of variable size.
		VariableCount = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT
	};

	using DescriptorFlags = tz::EnumField<DescriptorFlag>;

	/**
	 * @ingroup tz_gl_vk_descriptors
	 * Specifies creation flags for a descriptor set layout.
	 *
	 * See @ref DescriptorLayoutBuilder to help populate this structure.
	 */
	struct DescriptorLayoutInfo
	{
		/// Retrieve the number of bindings.
		std::size_t binding_count() const;
		/// Query as to whether the LogicalDevice `logical_device` is a valid device. That is, the device is not nullptr nor a null device.
		bool has_valid_device() const;
		/// BindingInfos might contain flags that are only optionally supported. This method returns a bool as to whether the LogicalDevice supports all these flags.
		bool device_supports_flags() const;

		struct BindingInfo
		{
			/// What is the type of the descriptor?
			DescriptorType type;
			/// How many are at this binding? If more than 1, we are an array.
			std::uint32_t count;
			/// Do we have any extra flags?
			DescriptorFlags flags;
		};
		/// List of all descriptor bindings in the layout.
		tz::BasicList<BindingInfo> bindings;
		/// LogicalDevice which will be creating the resultant @ref DescriptorLayout. This must not be null or a null LogicalDevice.
		const LogicalDevice* logical_device;
	};

	/**
	 * @ingroup tz_gl_vk_descriptors
	 * Specifies the types of resources that will be accessed by a graphics or compute pipeline via a @ref Shader.
	 */
	class DescriptorLayout
	{
	public:
		/**
		 * Construct a DescriptorLayout.
		 */
		DescriptorLayout(DescriptorLayoutInfo info);
		DescriptorLayout(const DescriptorLayout& copy) = delete;
		DescriptorLayout(DescriptorLayout&& move);
		~DescriptorLayout();

		DescriptorLayout& operator=(const DescriptorLayout& rhs) = delete;
		DescriptorLayout& operator=(DescriptorLayout&& rhs);

		/**
		 * Retrieve the number of bindings within the layout.
		 * @return Number of bindings.
		 */
		std::size_t binding_count() const;
		/**
		 * Retrieve the total number of descriptors in all of the bindings.
		 * @return Sum of all descriptor counts within the bindings.
		 */
		std::size_t descriptor_count() const;
		/**
		 * Retrieve the total number of descriptors of the given type in all of the bindings.
		 * @param type Descriptor type to retrieve the count of.
		 * @return Sum of all descriptor counts within the bindings matching `type`.
		 */
		std::size_t descriptor_count_of(DescriptorType type) const;
		/**
		 * Retrieve a read-only view into the bindings data for the layout.
		 * @return Span of structures containing information for each respective binding. The i'th element of the span is the binding with the binding-index `i`.
		 */
		std::span<const DescriptorLayoutInfo::BindingInfo> get_bindings() const;

		const LogicalDevice& get_device() const;

		using NativeType = VkDescriptorSetLayout;
		NativeType native() const;

		static DescriptorLayout null();
		bool is_null() const;
	private:
		DescriptorLayout();
		const DescriptorLayoutInfo& get_info() const;

		VkDescriptorSetLayout descriptor_layout;
		DescriptorLayoutInfo info;
	};

	/**
	 * @ingroup tz_gl_vk_descriptors
	 * Helper class to populate a @ref DescriptorLayoutInfo.
	 */
	class DescriptorLayoutBuilder
	{
	public:
		/**
		 * Create a builder starting with no bindings.
		 */
		DescriptorLayoutBuilder() = default;
		/**
		 * Create a builder based upon an existing layout.
		 */
		DescriptorLayoutBuilder(DescriptorLayoutInfo existing_info);
		/**
		 * Add a new binding. See @ref DescriptorLayoutInfo::BindingInfo for more information.
		 */
		DescriptorLayoutBuilder& with_binding(DescriptorLayoutInfo::BindingInfo binding);
		/**
		 * Retrieve the LogicalDevice which will be used to construct the resultant layout.
		 * @return Pointer to LogicalDevice. This is initially nullptr.
		 */
		const LogicalDevice* get_device() const;
		/**
		 * Set which LogicalDevice will be used to construct the resultant layout. Note that it is an error not to provide a valid LogicalDevice when creating the @ref DescriptorLayout.
		 * @param device LogicalDevice which will own the layout.
		 */
		void set_device(const LogicalDevice& device);
		/**
		 * Retrieve the info structure corresponding to this builder. This can be used to construct the @ref DescriptorLayout.
		 * @return Layout info structure.
		 */
		const DescriptorLayoutInfo& get_info() const;
		/**
		 * Create a new @ref DescriptorLayout based upon this builder.
		 * @return New DescriptorLayout.
		 */
		DescriptorLayout build() const;
		/**
		 * Undo all information, including bindings and device info back to default settings.
		 */
		void clear();
	private:
		DescriptorLayoutInfo info;
	};

	/**
	 * @ingroup tz_gl_vk_descriptors
	 * Specifies creation flags for a descriptor pool.
	 */
	struct DescriptorPoolInfo
	{
		/**
		 * Create a PoolInfo large enough such that `quantity` descriptor sets each matching `layout` can be allocated.
		 * @param descriptor_layout Layout from which the capacities of the pool will be specified.
		 * @param quantity Number of sets using `layout` which can fit in the pool.
		 */
		static DescriptorPoolInfo to_fit_layout(const DescriptorLayout& descriptor_layout, std::size_t quantity);
		/**
		 * Query as to whether the provided LogicalDevice is valid. This is the case if the LogicalDevice is not nullptr nor a null device.
		 */
		bool has_valid_device() const;

		/**
		 * Structure specifying limits for a DescriptorPool.
		 */
		struct PoolLimits
		{
			/// Map of the maximum number of descriptors for each type. If a type does not exist within the map, no such descriptors can be allocated from the pool.
			std::unordered_map<DescriptorType, std::uint32_t> limits;
			/// Maximum number of sets that can be allocated from the pool, regardless of layout.
			std::uint32_t max_sets = 0;
			/// True if any of the descriptors should be able to be updated after bind. This is true if bindless descriptors are created.
			bool supports_update_after_bind = false;
		};

		/// Specifies the limits for the created pool.
		PoolLimits limits;
		/// LogicalDevice which will be used to create the pool.
		const LogicalDevice* logical_device;
	};


	/**
	 * @ingroup tz_gl_vk_descriptors
	 * Represents a set of one or more descriptors.
	 */
	class DescriptorSet
	{
	public:
		/**
		 * Specifies information about writing to a specific element of the descriptor set via binding id.
		 */
		struct Write
		{
			/**
			 * Specifies information about having a descriptor refer to an existing @ref Buffer.
			 */
			struct BufferWriteInfo
			{
				/// Buffer to refer to. Must not be nullptr.
				const Buffer* buffer;
				/// Offset, in bytes, from the start of the buffer data.
				std::size_t buffer_offset;
				/// Number of bytes from the buffer to associate with the descriptor.
				std::size_t buffer_write_size;
			};

			/**
			 * Specifies information about having a descriptor refer to an existing @ref Image.
			 */
			struct ImageWriteInfo
			{
				/// Sampler to use. Must not be nullptr.
				const Sampler* sampler;
				/// ImageView to use. Must not be nullptr and must refer to a valid @ref Image.
				const ImageView* image_view;
			};
			using WriteInfo = std::variant<BufferWriteInfo, ImageWriteInfo>;

			/// Specifies which set we are working on. Must not be null.
			const DescriptorSet* set;
			/// Specifies the binding-id at which the descriptor/descriptor array will be written to.
			std::uint32_t binding_id;
			/// Zero or the element of the descriptor array we would like to start writes to.
			std::uint32_t array_element;
			/// List of writes to the descriptor array
			std::vector<WriteInfo> write_infos;
		};
		using WriteList = tz::BasicList<Write>;

		/**
		 * Request structure representing zero or more descriptor changes for this set.
		 *
		 * To create an edit request, see @ref DescriptorSet::make_edit_request.
		 */
		class EditRequest
		{
		public:
			/**
			 * Request that the buffer at the given binding at the provided array index refers to an existing @ref Buffer.
			 * @param binding_id Binding id to write a buffer to.
			 * @param buffer_write Struct containing information about which @ref Buffer is to be referred to by this descriptor, and optionally at a given subregion of the buffer.
			 * @param array_index Index of the descriptor array which this edit applies to. If this binding does not refer to a descriptor array, this must be zero. Default zero.
			 * @pre The descriptor or descriptor array at `binding_id` must be a 'buffery' @ref DescriptorType. This is either `UniformBuffer` or `StorageBuffer`.
			 * @pre If `array_index != 0`, then there must be a descriptor array at the given `binding_id` of size greater than or equal to `array_index`.
			 */
			void set_buffer(std::uint32_t binding_id, Write::BufferWriteInfo buffer_write, std::uint32_t array_index = 0);
			/**
			 * Request that the image at the given binding at the provided array index refers to an existing @ref Image.
			 * @param binding_id Binding id to write an image to.
			 * @param image_write Struct containing information about which @ref Image is to be referred to by this descriptor, and optionally a new @ref Sampler.
			 * @param array_index Index of the descriptor array which this edit applies to. If this binding does not refer to a descriptor array, this must be zero. Default zero.
			 * @pre The descriptor or descriptor array at `binding_id` must be a 'imagey' @ref DescriptorType. This is either `Image`, `ImageWithSampler`, or `StorageImage`.
			 * @pre If `array_index != 0`, then there must be a descriptor array at the given `binding_id` of size greater than or equal to `array_index`.
			 */
			void set_image(std::uint32_t binding_id, Write::ImageWriteInfo image_write, std::uint32_t array_index = 0);
			/**
			 * Retrieve a basic list of writes corresponding to all requested edits so far.
			 */
			DescriptorSet::WriteList to_write_list() const;
			/**
			 * Retrieve the @ref DescriptorSet which this request is intending to edit.
			 */
			const DescriptorSet& get_set() const;
			friend class DescriptorSet;
		private:
			EditRequest(DescriptorSet& set);

			DescriptorSet* set;
			DescriptorSet::WriteList write_info;
		};

		friend class DescriptorPool;
		using NativeType = VkDescriptorSet;
		NativeType native() const;

		/**
		 * Retrieve the @ref DescriptorLayout which this set matches.
		 * @return Layout of this DescriptorSet.
		 */
		const DescriptorLayout& get_layout() const;
		/**
		 * Retrieve an empty request for this set. You can use this to request changes to existing descriptors within this set.
		 * @note This only represents a *request* to make changes. To submit requests, this request must be passed to a @ref DescriptorPool::UpdateRequest.
		 */
		EditRequest make_edit_request();
	private:
		/// See @ref DescriptorPool.
		DescriptorSet(std::size_t set_id, const DescriptorLayout& layout, NativeType native);
		VkDescriptorSet set;
		std::uint32_t set_id;
		const DescriptorLayout* layout;
	};

	/**
	 * @ingroup tz_gl_vk_descriptors
	 * Represents storage for DescriptorSets.
	 */
	class DescriptorPool
	{
	public:
		/// Specifies information about a DescriptorPool allocation.
		struct Allocation
		{
			/// List of layouts for each set. When an allocation is performed, a list of DescriptorSets is returned. The list has size matching `set_layouts.length()` and the i'th element of the list will have layout matching `set_layouts[i]`.
			tz::BasicList<const DescriptorLayout*> set_layouts;
		};

		/// Specifies information about the resultant of an invocation to @ref DescriptorPool::allocate_sets
		struct AllocationResult
		{
			/// Returns true if the allocation was successful.
			bool success() const;

			/// Allocations can succeed, or fail in various ways.
			enum class AllocationResultType
			{
				/// - Allocation succeeded. Valid DescriptorSets available within the returned list.
				Success,
				/// - Allocation failed. Fatal error occurred. No way of recovery.
				FatalError,
				/// - Allocation failed. Pool technically had enough memory for the allocation, but it was fragmented.
				FragmentedPool,
				/// - Allocation failed. Pool did not have enough memory for the allocation.
				PoolOutOfMemory
			};

			/// List of allocated sets. If `type` == `AllocationResultType::Success`, this contains a list of valid DescriptorSets which can now be used.
			tz::BasicList<DescriptorSet> sets;
			/// Describes how the allocation went. Allocations can fail.
			AllocationResultType type;
		};

		/**
		 * Specifies a requests to update zero or more @ref DescriptorSets owned by an existing @ref DescriptorPool.
		 * - See @ref DescriptorPool::update_sets(UpdateRequest) for usage.
		 * - To create a request for a given pool, see @ref DescriptorPool::make_update_request
		 */
		class UpdateRequest
		{
		public:
			/**
			 * Add an edit for a @ref DescriptorSet.
			 */
			void add_set_edit(DescriptorSet::EditRequest set_edit);
			/**
			 * Retrieve all @ref DescriptorSet edit requests added so far.
			 */
			std::span<const DescriptorSet::EditRequest> get_set_edits() const;
			friend class DescriptorPool;
		private:
			UpdateRequest(DescriptorPool& pool);

			DescriptorPool* pool;
			std::vector<DescriptorSet::EditRequest> set_edits;
		};

		DescriptorPool(DescriptorPoolInfo info);
		DescriptorPool(const DescriptorPool& copy) = delete;
		DescriptorPool(DescriptorPool&& move);
		~DescriptorPool();

		DescriptorPool& operator=(const DescriptorPool& rhs) = delete;
		DescriptorPool& operator=(DescriptorPool&& rhs);

		/**
		 * Query as to whether an existing DescriptorSet was allocated from this pool.
		 * @return True if set belongs to this pool and is still allocated, otherwise false.
		 */
		bool contains(const DescriptorSet& set) const;

		/**
		 * Retrieve the LogicalDevice which was used to create the pool.
		 */
		const LogicalDevice& get_device() const;
		/**
		 * Allocate some DescriptorSets. See @ref Allocation for more info.
		 * @return Structure with resultant information, including newly-allocated DescriptorSets.
		 */
		AllocationResult allocate_sets(const Allocation& alloc);
		/**
		 * Retrieve an empty update request for this pool. You can fill this in and then pass to @ref DescriptorPool::update_sets(UpdateRequest) to edit descriptors within any owned sets.
		 */
		UpdateRequest make_update_request();
		/**
		 * Issue an update request to any DescriptorSets owned by this pool. See @ref UpdateRequest for details.
		 * See @ref DescriptorPool::make_update_request to retrieve an empty @ref UpdateRequest to make descriptor changes within owned sets.
		 */
		void update_sets(UpdateRequest update_request);
		/**
		 * Update some existing DescriptorSets. See @ref DescriptorSet::Write for more.
		 * @param writes List of DescriptorSet::Write to make changes to existing descriptors or descriptor arrays.
		 * @deprecated Creating WriteLists from scratch is really finnicky and easy to screw up. See @ref update_sets(DescriptorPool::UpdateRequest) as a better alternative.
		 */
		void update_sets(const DescriptorSet::WriteList& writes);
		/**
		 * Purge all DescriptorSets. The pool reclaims all memory from previously allocated DescriptorSets. This also means that all DescriptorSets allocated from this pool are now invalid.
		 */
		void clear();
	private:
		VkDescriptorPool pool;
		DescriptorPoolInfo info;
		std::vector<DescriptorSet::NativeType> allocated_set_natives;
	};
}

#endif // TOPAZ_GL_IMPL_BACKEND_VK2_DESCRIPTORS2_HPP
