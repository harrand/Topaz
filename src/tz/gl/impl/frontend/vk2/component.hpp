#ifndef TOPAZ_GL2_IMPL_FRONTEND_VK2_COMPONENT_HPP
#define TOPAZ_GL2_IMPL_FRONTEND_VK2_COMPONENT_HPP
#if TZ_VULKAN
#include "tz/gl/declare/image_format.hpp"
#include "tz/gl/api/component.hpp"
#include "tz/gl/impl/backend/vk2/buffer.hpp"
#include "tz/gl/impl/backend/vk2/image.hpp"

namespace tz::gl
{
	using namespace tz::gl;
	class BufferComponentVulkan : public IComponent
	{
	public:
		BufferComponentVulkan(IResource& resource, const vk2::LogicalDevice& ldev);
		virtual const IResource* get_resource() const final;
		virtual IResource* get_resource() final;
		// Satisfy BufferComponentType
		std::size_t size() const;
		void resize(std::size_t sz);

		const vk2::Buffer& vk_get_buffer() const;
		vk2::Buffer& vk_get_buffer();
		bool vk_is_descriptor_relevant() const;
	private:
		vk2::Buffer make_buffer(const vk2::LogicalDevice& ldev) const;

		IResource* resource;
		vk2::Buffer buffer;
	};
	static_assert(BufferComponentType<BufferComponentVulkan>);

	class ImageComponentVulkan : public IComponent
	{
	public:
		ImageComponentVulkan(IResource& resource, const vk2::LogicalDevice& ldev);
		virtual const IResource* get_resource() const final;
		virtual IResource* get_resource() final;
		// Satisfy ImageComponentType
		hdk::vec2ui get_dimensions() const;
		ImageFormat get_format() const;
		void resize(hdk::vec2ui new_dimensions);

		const vk2::Image& vk_get_image() const;
		vk2::Image& vk_get_image();
	private:
		vk2::Image make_image(const vk2::LogicalDevice& ldev) const;

		IResource* resource;
		vk2::Image image;
	};
	static_assert(ImageComponentType<ImageComponentVulkan>);
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL2_IMPL_FRONTEND_VK2_COMPONENT_HPP
