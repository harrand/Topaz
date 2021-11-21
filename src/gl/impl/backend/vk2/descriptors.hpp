#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_DESCRIPTORS2_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_DESCRIPTORS2_HPP
#include "core/containers/basic_list.hpp"
#include "gl/impl/backend/vk2/logical_device.hpp"

namespace tz::gl::vk2
{
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

	enum class DescriptorFlag
	{
		UpdateAfterBind = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
		UpdateUnusedWhilePending = VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT,
		PartiallyBound = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
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
}

#endif // TOPAZ_GL_IMPL_BACKEND_VK2_DESCRIPTORS2_HPP
