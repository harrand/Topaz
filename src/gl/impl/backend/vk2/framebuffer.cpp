#if TZ_VULKAN
#include "gl/impl/backend/vk2/framebuffer.hpp"

namespace tz::gl::vk2
{
	bool FramebufferInfo::valid() const
	{
		return this->render_pass != nullptr
			&& std::all_of(this->attachments.begin(), this->attachments.end(),
				[this](const ImageView* view_ptr) -> bool
				{
					return view_ptr->get_image().get_dimensions() == this->dimensions; 
				})
			&& !this->render_pass->get_device().is_null();
	}

	Framebuffer::Framebuffer(FramebufferInfo info):
	info(info),
	framebuffer(VK_NULL_HANDLE)
	{
		tz_assert(this->info.valid(), "FramebufferInfo was not valid. Please submit a bug report.");
		std::vector<ImageView::NativeType> view_natives(info.attachments.length());
		std::transform(info.attachments.begin(), info.attachments.end(), view_natives.begin(),
		[](const ImageView* view) -> ImageView::NativeType
		{
			return view->native();
		});

		VkFramebufferCreateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.renderPass = this->info.render_pass->native(),
			.attachmentCount = static_cast<std::uint32_t>(view_natives.size()),
			.pAttachments = view_natives.data(),
			.width = this->info.dimensions[0],
			.height = this->info.dimensions[1],
			.layers = 1
		};

		VkResult res = vkCreateFramebuffer(this->get_device().native(), &create, nullptr, &this->framebuffer);
		switch(res)
		{
			case VK_SUCCESS:
				// do nothing
			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				tz_error("Failed to create Framebuffer because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz_error("Failed to create Framebuffer because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
			break;
			default:
				tz_error("Failed to create Framebuffer but cannot determine why. Please submit a bug report.");
			break;
		}
	}

	Framebuffer::Framebuffer(Framebuffer&& move):
	info(),
	framebuffer(VK_NULL_HANDLE)
	{
		*this = std::move(move);
	}

	Framebuffer::~Framebuffer()
	{
		if(this->framebuffer != VK_NULL_HANDLE)
		{
			vkDestroyFramebuffer(this->get_device().native(), this->framebuffer, nullptr);
			this->framebuffer = VK_NULL_HANDLE;
		}
	}

	Framebuffer& Framebuffer::operator=(Framebuffer&& rhs)
	{
		std::swap(this->info, rhs.info);
		std::swap(this->framebuffer, rhs.framebuffer);
		return *this;
	}

	tz::Vec2ui Framebuffer::get_dimensions() const
	{
		return this->info.dimensions;
	}

	const RenderPass& Framebuffer::get_pass() const
	{
		tz_assert(this->info.render_pass != nullptr, "Framebuffer had no attached RenderPass. Please submit a bug report");
		return *this->info.render_pass;
	}

	const LogicalDevice& Framebuffer::get_device() const
	{
		return this->get_pass().get_device();
	}

	tz::BasicList<const ImageView*> Framebuffer::get_attachment_views() const
	{
		tz::BasicList<const ImageView*> views_const;
		views_const.resize(this->info.attachments.length());
		std::transform(this->info.attachments.begin(), this->info.attachments.end(), views_const.begin(), [](ImageView* view){return view;});
		return views_const;
	}

	tz::BasicList<ImageView*> Framebuffer::get_attachment_views()
	{
		return this->info.attachments;
	}

	Framebuffer::NativeType Framebuffer::native() const
	{
		return this->framebuffer;
	}
}

#endif // TZ_VULKAN
