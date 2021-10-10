#ifndef TOPAZ_GL_VK_DESCRIPTOR_SET_LAYOUT_HPP
#define TOPAZ_GL_VK_DESCRIPTOR_SET_LAYOUT_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk/logical_device.hpp"
#include "gl/impl/backend/vk/pipeline/shader_stage.hpp"
#include <cstdint>

namespace tz::gl::vk
{
	enum class DescriptorType
	{
		UniformBuffer,
		ShaderStorageBuffer,
		CombinedImageSampler
	};

	class LayoutBuilder
	{
	public:
		LayoutBuilder() = default;
		std::uint32_t add(DescriptorType type, pipeline::ShaderTypeField relevant_stages);
		VkDescriptorSetLayoutBinding operator[](std::size_t index) const;
		VkDescriptorSetLayoutCreateInfo native() const;

		std::size_t size() const;
	private:
		std::vector<VkDescriptorType> binding_types;
		std::vector<VkShaderStageFlags> binding_relevant_shader_stages;
		mutable std::vector<VkDescriptorSetLayoutBinding> bindings;
	};

	class DescriptorSetLayout
	{
	public:
		DescriptorSetLayout(const LogicalDevice& device, const LayoutBuilder& builder);
		DescriptorSetLayout(const DescriptorSetLayout& copy) = delete;
		DescriptorSetLayout(DescriptorSetLayout&& move);
		~DescriptorSetLayout();

		DescriptorSetLayout& operator=(const DescriptorSetLayout& rhs) = delete;
		DescriptorSetLayout& operator=(DescriptorSetLayout&& rhs);

		VkDescriptorSetLayout native() const;
	private:
		VkDescriptorSetLayout layout;
		const LogicalDevice* device;
	};

	using DescriptorSetLayoutRefs = std::vector<std::reference_wrapper<const DescriptorSetLayout>>;
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_DESCRIPTOR_SET_LAYOUT_HPP