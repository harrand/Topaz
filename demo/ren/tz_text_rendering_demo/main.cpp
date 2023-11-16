#include "tz/tz.hpp"
#include "tz/gl/device.hpp"
#include "tz/io/ttf.hpp"

#include "tz/core/imported_text.hpp"
#include ImportedTextHeader(ProggyClean, ttf)

int main()
{
	tz::initialise({.name = "tz_text_rendering_demo"});
	{
		tz::io::ttf ttf = tz::io::ttf::from_memory(ImportedTextData(ProggyClean, ttf));

		tz::begin_frame();
		tz::gl::get_device().render();
		tz::end_frame();
	}
	tz::terminate();
}