#include "tz/gl/output.hpp"

namespace tz::gl
{
	ImageOutput::ImageOutput(ImageOutputInfo info):
	colour_attachments(info.colours.length(), nullptr),
	depth_attachment(info.depth != nullptr ? static_cast<ImageComponent*>(info.depth) : nullptr)
	{
		hdk::assert(std::all_of(info.colours.begin(), info.colours.end(), [](IComponent* comp)
		{
			return comp != nullptr && comp->get_resource() != nullptr && comp->get_resource()->get_type() == ResourceType::Image;
		}), "Provided at least one IComponent to an ImageOutput colour attachment which was not an ImageResource. Please submit a bug report.");

		hdk::assert(std::all_of(info.colours.begin(), info.colours.end(), [](IComponent* comp)
		{
			return comp != nullptr && comp->get_resource() != nullptr && comp->get_resource()->get_flags().contains(ResourceFlag::RendererOutput);
		}), "ImageOutput contains colour attachments, but one or more of them is missing ResourceFlag::RendererOutput. An image resource cannot be an image output attachment unless it contains this flag.");

		if(this->depth_attachment != nullptr)
		{
			hdk::assert(info.depth->get_resource() != nullptr && info.depth->get_resource()->get_type() == ResourceType::Image, "Provided an IComponent to an ImageOutput depth attachment which was not an ImageResource. Please submit a bug report.");
		}
		std::transform(info.colours.begin(), info.colours.end(), this->colour_attachments.begin(),
		[](IComponent* comp)
		{
			return static_cast<ImageComponent*>(comp);
		});
	}

	std::size_t ImageOutput::colour_attachment_count() const
	{
		return this->colour_attachments.size();
	}

	bool ImageOutput::has_depth_attachment() const
	{
		return this->depth_attachment != nullptr;
	}

	const ImageComponent& ImageOutput::get_colour_attachment(std::size_t colour_attachment_idx) const
	{
		return *this->colour_attachments[colour_attachment_idx];
	}

	ImageComponent& ImageOutput::get_colour_attachment(std::size_t colour_attachment_idx)
	{
		return *this->colour_attachments[colour_attachment_idx];
	}

	const ImageComponent& ImageOutput::get_depth_attachment() const
	{
		hdk::assert(this->has_depth_attachment(), "Requested ImageOutput depth attachment, but there isn't one. Please submit a bug report.");
		return *this->depth_attachment;
	}

	ImageComponent& ImageOutput::get_depth_attachment()
	{
		hdk::assert(this->has_depth_attachment(), "Requested ImageOutput depth attachment, but there isn't one. Please submit a bug report.");
		return *this->depth_attachment;
	}

	WindowOutput::WindowOutput(const tz::wsi::window& window):
	wnd(&window){}

	const tz::wsi::window& WindowOutput::get_window() const
	{
		return *this->wnd;
	}
}
