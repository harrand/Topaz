#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_DESCRIPTORS_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_DESCRIPTORS_HPP
#include "gl/impl/backend/vk2/logical_device.hpp"
#include <variant>

namespace tz::gl::vk2
{
	/**
	 * @ingroup tz_gl_vk_descriptors
	 * Descriptors can be created/used in multiple contexts.
	 */
	enum class DescriptorContext
	{
		/// - Old-school, bindful descriptor behaviour.
		Classic,
		/// - Bindless descriptor behaviour. Requires the @ref PhysicalDevice to support @ref DeviceExtension::Bindless.
		Bindless
	};

	/**
	 * @ingroup tz_gl_vk_descriptors
	 */
	enum class DescriptorType
	{
		Sampler = VK_DESCRIPTOR_TYPE_SAMPLER,
		ImageWithSampler = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		Image = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		StorageImage = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,

		UniformBuffer = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		StorageBuffer = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
	};

	/**
	 * @ingroup tz_gl_vk_descriptors
	 * Specifies creation flags for descriptor set layout bindings.
	 */
	struct DescriptorLayoutBindlessFlagsInfo
	{
		/// Flags for each descriptor set layout binding (of which there are likely to be many for Bindless Descriptors).
		tz::BasicList<VkDescriptorBindingFlags> binding_flags;

		using NativeType = VkDescriptorSetLayoutBindingFlagsCreateInfo;
		NativeType native() const;
	};

	/**
	 * @ingroup tz_gl_vk_descriptors
	 * Specifies creation flags for a descriptor set layout.
	 *
	 * Special helper classes exist to populate this structure. See:
	 * - @ref DescriptorLayoutBuilder to produce classical layouts.
	 * - @ref DescriptorLayoutBuilderBindless to produce bindless layouts.
	 */
	struct DescriptorLayoutInfo
	{
		/// Specifies the context in which the Descriptors should be created.
		DescriptorContext context;
		/// List of all descriptors in the layout.
		tz::BasicList<VkDescriptorSetLayoutBinding> bindings;
		/// If we're in a bindless context (see @ref DescriptorLayoutInfo::context) then this must have a value which specifies the flags for each element in each descriptor binding.
		std::optional<DescriptorLayoutBindlessFlagsInfo> maybe_bindless_flags;
		/// LogicalDevice which will be creating the resultant @ref DescriptorLayout. This must not be null or a null LogicalDevice.
		const LogicalDevice* logical_device;
	};

	/**
	 * @ingroup tz_gl_vk_descriptors
	 * Intuitive builder interface for creating a classical @ref DescriptorLayoutInfo.
	 * If you wish to produce bindless layouts, see @ref DescriptorLayoutBuilderBindless.
	 */
	class DescriptorLayoutBuilder
	{
	public:
		/**
		 * Initialise the builder with the @ref LogicalDevice that will end up creating the @ref DescriptorLayout.
		 * @param logical_device Device with which the layout will be created. This must not be null nor a null device.
		 */
		DescriptorLayoutBuilder(const LogicalDevice& logical_device);
		/**
		 * Add a binding containing a single descriptor of the given type to the set layout.
		 * @param desc Type of the descriptor.
		 */
		DescriptorLayoutBuilder& with_descriptor(DescriptorType desc);
		/**
		 * Build the resultant layout information.
		 * @return Structure which can be used to construct a @ref DescriptorLayout.
		 */
		DescriptorLayoutInfo build() const;
	private:
		const LogicalDevice* logical_device;
		// Each binding has one descriptor
		std::vector<DescriptorType> descriptors;
	};

	/**
	 * @ingroup tz_gl_vk_descriptors
	 * A specialised @ref DescriptorLayoutBuilder to produce a layout utilising @ref DeviceFeature::BindlessDescriptors.
	 */
	class DescriptorLayoutBuilderBindless
	{
	public:
		/**
		 * Initialise the builder based upon the given LogicalDevice.
		 * @pre `logical_device` must have @ref DeviceFeature::BindlessDescriptors enabled.
		 */
		DescriptorLayoutBuilderBindless(const LogicalDevice& logical_device);
		/**
		 * Add a binding containing an arbitrary number of descriptors of the given type to the set layout.
		 * @param desc Type of the descriptor.
		 * @param descriptor_count Number of descriptors in the binding.
		 */
		DescriptorLayoutBuilderBindless& with_descriptor(DescriptorType desc, std::size_t descriptor_count);
		/**
		 * Build the resultant layout information.
		 * @return Structure which can be used to construct a @ref DescriptorLayout.
		 */
		DescriptorLayoutInfo build() const;
	private:
		struct DescriptorLayoutElementInfo
		{
			DescriptorType type;
			std::uint32_t count;
		};

		const LogicalDevice* logical_device;
		std::vector<DescriptorLayoutElementInfo> descriptors;
	};

	/**
	 * @ingroup tz_gl_vk_descriptors
	 * Specifies the types of resources that will be accessed by a graphics or compute pipeline via a @ref Shader.
	 */
	class DescriptorLayout
	{
	public:
		DescriptorLayout(DescriptorLayoutInfo info);
		DescriptorLayout(const DescriptorLayout& copy) = delete;
		DescriptorLayout(DescriptorLayout&& move);
		~DescriptorLayout();

		DescriptorLayout& operator=(const DescriptorLayout& rhs) = delete;
		DescriptorLayout& operator=(DescriptorLayout&& rhs);

		const DescriptorLayoutInfo& get_info() const;

		using NativeType = VkDescriptorSetLayout;
		NativeType native() const;

		static DescriptorLayout null();
		bool is_null() const;
	private:
		DescriptorLayout();

		VkDescriptorSetLayout descriptor_layout;
		DescriptorLayoutInfo info;
		const LogicalDevice* logical_device;
	};

	class DescriptorSet
	{
	public:
		friend class DescriptorPool;
		using NativeType = VkDescriptorSet;
		NativeType native() const;

		const DescriptorLayout& get_layout() const;
	private:
		/// See @ref DescriptorPool.
		DescriptorSet(std::size_t set_id, const DescriptorLayout& layout, NativeType native);
		VkDescriptorSet set;
		std::uint32_t set_id;
		const DescriptorLayout* layout;
	};

	/**
	 * @ingroup tz_gl_vk_descriptors
	 * Specifies creation flags for a DescriptorPool.
	 * A special helper function exists to populate this structure. See:
	 * - @ref create_pool_for_layout
	 */
	struct DescriptorPoolInfo
	{
		using PoolSize = VkDescriptorPoolSize;

		/// Owner device. This must not be nullptr or a null device.
		const LogicalDevice* logical_device;
		/// Describes the context of the descriptors that will be contained within the pool.
		DescriptorContext context;
		/// Maximum number of descriptor sets.
		std::size_t maximum_descriptor_set_count;
		/// List of information about limits for each DescriptorType.
		tz::BasicList<PoolSize> pool_sizes;
	};

	/**
	 * @ingroup tz_gl_vk_descriptors
	 * Specify creation flags for a DescriptorPool large enough to contain `number_of_sets` that all use the same `layout_info`.
	 */
	DescriptorPoolInfo create_pool_for_layout(const DescriptorLayoutInfo& layout_info, std::size_t number_of_sets);

	/**
	 * @ingroup tz_gl_descriptors
	 * Maintains a pool of descriptors, from which descriptor sets are allocated.
	 */
	class DescriptorPool
	{
	public:
		struct AllocateInfo
		{
			tz::BasicList<const DescriptorLayout*> set_layouts;
		};

		struct UpdateInfo
		{
			struct Write
			{
				using VariantType = std::variant<VkDescriptorBufferInfo, VkDescriptorImageInfo>;
				const DescriptorSet* set;
				std::uint32_t binding_id;
				std::vector<VariantType> write_info;
			};

			tz::BasicList<Write> writes;
		};

		DescriptorPool(const DescriptorPoolInfo& info);
		DescriptorPool(const DescriptorPool& copy) = delete;
		DescriptorPool(DescriptorPool&& move);
		~DescriptorPool();

		DescriptorPool& operator=(const DescriptorPool& rhs) = delete;
		DescriptorPool& operator=(DescriptorPool&& rhs);

		tz::BasicList<DescriptorSet> allocate_sets(const AllocateInfo& alloc);
		void update_sets(const UpdateInfo& update);
		void clear();

		using NativeType = VkDescriptorPool;
		NativeType native() const;
	private:
		VkDescriptorPool pool;
		DescriptorContext context;
		const LogicalDevice* logical_device;
		std::vector<DescriptorSet::NativeType> allocated_set_natives;
	};
}

#endif // TOPAZ_GL_IMPL_BACKEND_VK2_DESCRIPTORS_HPP
