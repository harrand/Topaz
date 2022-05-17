#ifndef TOPAZ_GL2_OUTPUT_HPP
#define TOPAZ_GL2_OUTPUT_HPP
#include "core/window.hpp"
#include "gl/api/output.hpp"
#include "gl/component.hpp"

namespace tz::gl
{
	struct ImageOutputInfo
	{
		tz::BasicList<IComponent*> colours;
		IComponent* depth;
	};

	class ImageOutput : public IOutput
	{
	public:
		ImageOutput(ImageOutputInfo info);
		constexpr virtual OutputTarget get_target() const override
		{
			return OutputTarget::OffscreenImage;
		}

		std::size_t colour_attachment_count() const;
		bool has_depth_attachment() const;

		const ImageComponent& get_colour_attachment(std::size_t colour_attachment_idx) const;
		ImageComponent& get_colour_attachment(std::size_t colour_attachment_idx);

	private:
		std::vector<ImageComponent*> colour_attachments;
		ImageComponent* depth_attachment;
	};

	class WindowOutput : public IOutput
	{
	public:
		WindowOutput(const tz::Window& window);
		constexpr virtual OutputTarget get_target() const override
		{
			return OutputTarget::Window;
		}

		const tz::Window& get_window() const;
	private:
		const tz::Window* wnd;
	};
}

#endif // TOPAZ_GL2_OUTPUT_HPP
