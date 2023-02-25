#include "tz/tz.hpp"
#include "tz/gl/impl/opengl/detail/tz_opengl.hpp"
#include "tz/gl/impl/opengl/detail/buffer.hpp"
#include <numeric>

tz::gl::ogl2::buffer make_dynamic_copy(const tz::gl::ogl2::buffer& buf)
{
	using namespace tz::gl::ogl2;
	buffer cpy
	{{
		.target = buf.get_target(),
		.residency = buffer_residency::dynamic,
		.size_bytes = buf.size()
	}};
	buffer_helper::copy(buf, cpy);
	return cpy;
}

bool buffer_data_matches(const tz::gl::ogl2::buffer& b1, const tz::gl::ogl2::buffer& b2)
{
	// If any of them is static, we need to make a buffer copy.
	using namespace tz::gl::ogl2;
	buffer b1_dyn = make_dynamic_copy(b1);
	buffer b2_dyn = make_dynamic_copy(b2);
	std::span<const std::byte> b1_bytes = b1_dyn.map_as<const std::byte>();
	std::span<const std::byte> b2_bytes = b2_dyn.map_as<const std::byte>();
	return std::equal(b1_bytes.begin(), b1_bytes.end(), b2_bytes.begin());
}

void create_empty_buffer()
{
	using namespace tz::gl::ogl2;
	buffer static_buf
	{{
		.target = buffer_target::uniform,
		.residency = buffer_residency::static_fixed,
		.size_bytes = 1
	}};
}

void semantics()
{
	using namespace tz::gl::ogl2;
	buffer buf1
	{{
		.target = buffer_target::uniform,
		.residency = buffer_residency::static_fixed,
		.size_bytes = 1
	}};
	buffer buf2
	{{
		.target = buffer_target::shader_storage,
		.residency = buffer_residency::dynamic,
		.size_bytes = 64
	}};
	buffer buf3 = std::move(buf1);
	buffer buf4 = buffer::null();
	buf4 = std::move(buf3);
	buf3 = std::move(buf2);
}

void buffer_copies()
{
	using namespace tz::gl::ogl2;
	buffer b1
	{{
		.target = buffer_target::uniform,
		.residency = buffer_residency::static_fixed,
		.size_bytes = 64
	}};

	buffer b2
	{{
		.target = buffer_target::uniform,
		.residency = buffer_residency::dynamic,
		.size_bytes = 64
	}};
	// b1 = {???}
	{
		std::span<float> b2_data = b2.map_as<float>();
		std::iota(b2_data.begin(), b2_data.end(), 0.0f);
	}
	// b2 = {0.0f, 1.0f, 2.0f, ...}
	tz::assert(!buffer_data_matches(b1, b2), "buffer data which are set to not have matching data store contents seem to have matching values.");
	// Now set b1 to iota aswell.
	{
		buffer_helper::copy(b1, b2);
	}
	tz::assert(buffer_data_matches(b1, b2), "buffer data which are set to have matching data store contents seem to not have matching values.");
}

int main()
{
	tz::initialise
	({
		.name = "ogl_buffer_test",
		.flags = {tz::application_flag::window_hidden}
	});
	{
		create_empty_buffer();
		semantics();
		buffer_copies();
	}
	tz::terminate();
	return 0;
}
