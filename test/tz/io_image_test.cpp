#include "tz/topaz.hpp"
#include "tz/os/file.hpp"
#include "tz/io/image.hpp"

#include "tz/main.hpp"
int tz_main()
{
	std::string imgdata = tz_must(tz::os::read_file("./files/img.png"));
	std::span<const std::byte> span{reinterpret_cast<std::byte*>(imgdata.data()), imgdata.size()};
	tz::io::image_header imghdr = tz_must(tz::io::image_info(span));
	std::vector<std::byte> data(imghdr.data_size_bytes);
	tz::io::parse_image(span, data);

	return 0;
}