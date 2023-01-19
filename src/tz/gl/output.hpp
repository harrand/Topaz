#ifndef TOPAZ_GL2_OUTPUT_HPP
#define TOPAZ_GL2_OUTPUT_HPP
#include "tz/core/containers/basic_list.hpp"
#include "tz/wsi/window.hpp"
#include "tz/gl/api/output.hpp"
#include "tz/gl/component.hpp"

namespace tz::gl
{
	struct ImageOutputInfo
	{
		tz::BasicList<icomponent*> colours;
		icomponent* depth = nullptr;
	};

	class ImageOutput final : public ioutput
	{
	public:
		ImageOutput(ImageOutputInfo info);
		constexpr virtual output_target get_target() const override
		{
			return output_target::offscreen_image;
		}

		virtual std::unique_ptr<ioutput> unique_clone() const override
		{
			return std::make_unique<ImageOutput>(*this);
		}

		std::size_t colour_attachment_count() const;
		bool has_depth_attachment() const;

		const image_component& get_colour_attachment(std::size_t colour_attachment_idx) const;
		image_component& get_colour_attachment(std::size_t colour_attachment_idx);
		
		const image_component& get_depth_attachment() const;
		image_component& get_depth_attachment();
	private:
		std::vector<image_component*> colour_attachments;
		image_component* depth_attachment;
	};

	class WindowOutput final : public ioutput
	{
	public:
		WindowOutput(const tz::wsi::window& window);
		constexpr virtual output_target get_target() const override
		{
			return output_target::window;
		}

		virtual std::unique_ptr<ioutput> unique_clone() const override
		{
			return std::make_unique<WindowOutput>(*this);
		}

		const tz::wsi::window& get_window() const;
	private:
		const tz::wsi::window* wnd;
	};
}

#endif // TOPAZ_GL2_OUTPUT_HPP
