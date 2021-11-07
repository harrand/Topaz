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
	 */
	struct DescriptorLayoutInfo
	{
		/// Specifies the context in which the Descriptors should be created.
		DescriptorContext context;
		/// List of all descriptors in the layout.
		tz::BasicList<VkDescriptorSetLayoutBinding> bindings;
		/// If we're in a bindless context (see @ref DescriptorLayoutInfo::context) then this must have a value which specifies the flags for each element in each descriptor binding.
		std::optional<VkDescriptorSetLayoutBindingFlagsCreateInfo> maybe_bindless_flags;
		/// LogicalDevice which will be creating the resultant @ref DescriptorLayout. This must not be null or a null LogicalDevice.
		const LogicalDevice* logical_device;

		using NativeType = VkDescriptorSetLayoutCreateInfo;
		NativeType native() const;
	};

	/**
	 * @ingroup tz_gl_vk_descriptors
	 * Intuitive builder interface for creating a @ref DescriptorLayoutInfo.
	 * @note DescriptorLayoutInfos for @ref DescriptorContext::Bindless are not yet implemented.
	 */
	class DescriptorLayoutBuilder
	{
	public:
		/**
		 * Initialise the builder with the @ref LogicalDevice that will end up creating the @ref DescriptorLayout.
		 * @param logical_device Device with which the layout will be created. This must not be null nor a null device.
		 */
		DescriptorLayoutBuilder(const LogicalDevice& logical_device);
		DescriptorLayoutBuilder& with_descriptor(DescriptorType desc);
		/**
		 * Build the resultant layout information.
		 * @return Structure which can be used to construct a @ref DescriptorLayout.
		 */
		DescriptorLayoutInfo build() const;
	private:
		const LogicalDevice* logical_device;
		std::vector<DescriptorType> descriptors;
	};

	/**
	 * @ingroup tz_gl_vk_descriptors
	 * Specifies the types of resources that will be accessed by a graphics or compute pipeline via a @ref Shader.
	 */
	class DescriptorLayout
	{
	public:
		DescriptorLayout(const DescriptorLayoutInfo& info);
		DescriptorLayout(const DescriptorLayout& copy) = delete;
		DescriptorLayout(DescriptorLayout&& move);
		~DescriptorLayout();

		DescriptorLayout& operator=(const DescriptorLayout& rhs) = delete;
		DescriptorLayout& operator=(DescriptorLayout&& rhs);

		using NativeType = VkDescriptorSetLayout;
		NativeType native() const;
	private:
		VkDescriptorSetLayout descriptor_layout;
		const LogicalDevice* logical_device;
	};
}

#endif // TOPAZ_GL_IMPL_BACKEND_VK2_DESCRIPTORS_HPP
