#include "tz/gl/output.hpp"

namespace tz::gl
{
	image_output::image_output(image_output_info info):
	colour_attachments(info.colours.length(), nullptr),
	depth_attachment(info.depth != nullptr ? static_cast<image_component*>(info.depth) : nullptr)
	{
		hdk::assert(std::all_of(info.colours.begin(), info.colours.end(), [](icomponent* comp)
		{
			return comp != nullptr && comp->get_resource() != nullptr && comp->get_resource()->get_type() == resource_type::image;
		}), "Provided at least one icomponent to an image_output colour attachment which was not an image_resource. Please submit a bug report.");

		hdk::assert(std::all_of(info.colours.begin(), info.colours.end(), [](icomponent* comp)
		{
			return comp != nullptr && comp->get_resource() != nullptr && comp->get_resource()->get_flags().contains(resource_flag::renderer_output);
		}), "image_output contains colour attachments, but one or more of them is missing resource_flag::renderer_output. An image resource cannot be an image output attachment unless it contains this flag.");

		if(this->depth_attachment != nullptr)
		{
			hdk::assert(info.depth->get_resource() != nullptr && info.depth->get_resource()->get_type() == resource_type::image, "Provided an icomponent to an image_output depth attachment which was not an image_resource. Please submit a bug report.");
		}
		std::transform(info.colours.begin(), info.colours.end(), this->colour_attachments.begin(),
		[](icomponent* comp)
		{
			return static_cast<image_component*>(comp);
		});
	}

	std::size_t image_output::colour_attachment_count() const
	{
		return this->colour_attachments.size();
	}

	bool image_output::has_depth_attachment() const
	{
		return this->depth_attachment != nullptr;
	}

	const image_component& image_output::get_colour_attachment(std::size_t colour_attachment_idx) const
	{
		return *this->colour_attachments[colour_attachment_idx];
	}

	image_component& image_output::get_colour_attachment(std::size_t colour_attachment_idx)
	{
		return *this->colour_attachments[colour_attachment_idx];
	}

	const image_component& image_output::get_depth_attachment() const
	{
		hdk::assert(this->has_depth_attachment(), "Requested image_output depth attachment, but there isn't one. Please submit a bug report.");
		return *this->depth_attachment;
	}

	image_component& image_output::get_depth_attachment()
	{
		hdk::assert(this->has_depth_attachment(), "Requested image_output depth attachment, but there isn't one. Please submit a bug report.");
		return *this->depth_attachment;
	}

	window_output::window_output(const tz::wsi::window& window):
	wnd(&window){}

	const tz::wsi::window& window_output::get_window() const
	{
		return *this->wnd;
	}
}
