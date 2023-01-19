#ifndef TOPAZ_GL2_IMPL_FRONTEND_VK2_COMPONENT_HPP
#define TOPAZ_GL2_IMPL_FRONTEND_VK2_COMPONENT_HPP
#if TZ_VULKAN
#include "tz/gl/declare/image_format.hpp"
#include "tz/gl/api/component.hpp"
#include "tz/gl/impl/vulkan/detail/buffer.hpp"
#include "tz/gl/impl/vulkan/detail/image.hpp"

namespace tz::gl
{
	using namespace tz::gl;
	class buffer_component_vulkan : public icomponent
	{
	public:
		buffer_component_vulkan(iresource& resource);
		virtual const iresource* get_resource() const final;
		virtual iresource* get_resource() final;
		// Satisfy buffer_component_type
		std::size_t size() const;
		void resize(std::size_t sz);

		const vk2::Buffer& vk_get_buffer() const;
		vk2::Buffer& vk_get_buffer();
		bool vk_is_descriptor_relevant() const;
	private:
		vk2::Buffer make_buffer() const;

		iresource* resource;
		vk2::Buffer buffer;
	};
	static_assert(buffer_component_type<buffer_component_vulkan>);

	class image_component_vulkan : public icomponent
	{
	public:
		image_component_vulkan(iresource& resource);
		virtual const iresource* get_resource() const final;
		virtual iresource* get_resource() final;
		// Satisfy image_component_type
		tz::vec2ui get_dimensions() const;
		image_format get_format() const;
		void resize(tz::vec2ui new_dimensions);

		const vk2::Image& vk_get_image() const;
		vk2::Image& vk_get_image();
	private:
		vk2::Image make_image() const;

		iresource* resource;
		vk2::Image image;
	};
	static_assert(image_component_type<image_component_vulkan>);
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL2_IMPL_FRONTEND_VK2_COMPONENT_HPP
