#include "core/tz.hpp"
#include "gl/impl/backend/ogl2/tz_opengl.hpp"
#include "gl/impl/backend/ogl2/buffer.hpp"
#include <numeric>

tz::gl::ogl2::Buffer make_dynamic_copy(const tz::gl::ogl2::Buffer& buf)
{
	using namespace tz::gl::ogl2;
	Buffer cpy
	{{
		.target = buf.get_target(),
		.residency = BufferResidency::Dynamic,
		.size_bytes = buf.size()
	}};
	buffer::copy(buf, cpy);
	return cpy;
}

bool buffer_data_matches(const tz::gl::ogl2::Buffer& b1, const tz::gl::ogl2::Buffer& b2)
{
	// If any of them is static, we need to make a buffer copy.
	using namespace tz::gl::ogl2;
	Buffer b1_dyn = make_dynamic_copy(b1);
	Buffer b2_dyn = make_dynamic_copy(b2);
	std::span<const std::byte> b1_bytes = b1_dyn.map_as<const std::byte>();
	std::span<const std::byte> b2_bytes = b2_dyn.map_as<const std::byte>();
	return std::equal(b1_bytes.begin(), b1_bytes.end(), b2_bytes.begin());
}

void create_empty_buffer()
{
	using namespace tz::gl::ogl2;
	Buffer static_buf
	{{
		.target = BufferTarget::Uniform,
		.residency = BufferResidency::Static,
		.size_bytes = 0
	}};
}

void semantics()
{
	using namespace tz::gl::ogl2;
	Buffer buf1
	{{
		.target = BufferTarget::Uniform,
		.residency = BufferResidency::Static
	}};
	Buffer buf2
	{{
		.target = BufferTarget::ShaderStorage,
		.residency = BufferResidency::Dynamic,
		.size_bytes = 64
	}};
	Buffer buf3 = std::move(buf1);
	Buffer buf4 = Buffer::null();
	buf4 = std::move(buf3);
	buf3 = std::move(buf2);
}

void buffer_copies()
{
	using namespace tz::gl::ogl2;
	Buffer b1
	{{
		.target = BufferTarget::Uniform,
		.residency = BufferResidency::Static,
		.size_bytes = 64
	}};

	Buffer b2
	{{
		.target = BufferTarget::Uniform,
		.residency = BufferResidency::Dynamic,
		.size_bytes = 64
	}};
	// b1 = {???}
	{
		std::span<float> b2_data = b2.map_as<float>();
		std::iota(b2_data.begin(), b2_data.end(), 0.0f);
	}
	// b2 = {0.0f, 1.0f, 2.0f, ...}
	tz_assert(!buffer_data_matches(b1, b2), "Buffer data which are set to not have matching data store contents seem to have matching values.");
	// Now set b1 to iota aswell.
	{
		buffer::copy(b1, b2);
	}
	tz_assert(buffer_data_matches(b1, b2), "Buffer data which are set to have matching data store contents seem to not have matching values.");
}

int main()
{
	tz::GameInfo game{"ogl_buffer_test", tz::Version{1, 0, 0}, tz::info()};
	tz::initialise(game, tz::ApplicationType::HiddenWindowApplication);
	{
		create_empty_buffer();
		semantics();
		buffer_copies();
	}
	tz::terminate();
	return 0;
}
