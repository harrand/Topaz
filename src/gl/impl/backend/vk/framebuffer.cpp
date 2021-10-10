#if TZ_VULKAN
#include "gl/impl/backend/vk/framebuffer.hpp"
#include "core/assert.hpp"
#include <array>

namespace tz::gl::vk
{
	Framebuffer::Framebuffer(const RenderPass& render_pass, const ImageView& col_view, VkExtent2D dimensions):
	frame_buffer(VK_NULL_HANDLE),
	colour_view(&col_view),
	depth_view(nullptr),
	device(&render_pass.get_device())
	{
		auto img_view_native = col_view.native();
		VkFramebufferCreateInfo create{};
		create.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		create.renderPass = render_pass.native();
		create.attachmentCount = 1;
		create.pAttachments = &img_view_native;
		create.width = dimensions.width;
		create.height = dimensions.height;
		create.layers = 1;

		auto res = vkCreateFramebuffer(this->device->native(), &create, nullptr, &this->frame_buffer);
		tz_assert(res == VK_SUCCESS, "Failed to create Framebuffer");
	}

	Framebuffer::Framebuffer(const RenderPass& render_pass, const ImageView& col_view, const ImageView& depth_view, VkExtent2D dimensions):
	frame_buffer(VK_NULL_HANDLE),
	colour_view(&col_view),
	depth_view(&depth_view),
	device(&render_pass.get_device())
	{
		std::array<VkImageView, 2> img_view_natives{col_view.native(), depth_view.native()};
		VkFramebufferCreateInfo create{};
		create.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		create.renderPass = render_pass.native();
		create.attachmentCount = img_view_natives.size();
		create.pAttachments = img_view_natives.data();
		create.width = dimensions.width;
		create.height = dimensions.height;
		create.layers = 1;

		auto res = vkCreateFramebuffer(this->device->native(), &create, nullptr, &this->frame_buffer);
		tz_assert(res == VK_SUCCESS, "Failed to create Framebuffer");
	}

	Framebuffer::Framebuffer(Framebuffer&& move):
	frame_buffer(VK_NULL_HANDLE),
	colour_view(nullptr),
	depth_view(nullptr),
	device(nullptr)
	{
		*this = std::move(move);
	}

	Framebuffer::~Framebuffer()
	{
		if(this->frame_buffer != VK_NULL_HANDLE)
		{
			vkDestroyFramebuffer(this->device->native(), this->frame_buffer, nullptr);
			this->frame_buffer = VK_NULL_HANDLE;
		}
	}

	Framebuffer& Framebuffer::operator=(Framebuffer&& rhs)
	{
		std::swap(this->frame_buffer, rhs.frame_buffer);
		std::swap(this->colour_view, rhs.colour_view);
		std::swap(this->depth_view, rhs.depth_view);
		std::swap(this->device, rhs.device);
		return *this;
	}

	const ImageView& Framebuffer::get_colour_view() const
	{
		return *this->colour_view;
	}

	const ImageView* Framebuffer::get_depth_view() const
	{
		return this->depth_view;
	}
	
	VkFramebuffer Framebuffer::native() const
	{
		return this->frame_buffer;
	}

}

#endif // TZ_VULKAN