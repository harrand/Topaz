#include "tz/core/tz.hpp"
#include "tz/core/assert.hpp"
#include "tz/core/report.hpp"
#include "tz/gl/resource.hpp"

//--------------------------------------------------------------------------------------------------
void null_buffer()
{
	tz_assert(tz::gl::BufferResource::null().is_null(), "Null buffer is not a null buffer");
}

//--------------------------------------------------------------------------------------------------
void null_image()
{
	tz_assert(tz::gl::ImageResource::null().is_null(), "Null image is not a null image");
}


//--------------------------------------------------------------------------------------------------
void basic_buffer()
{
	// Check that BufferResource::from_one returns expected data size.
	{
		auto buf = tz::gl::BufferResource::from_one(5u);
		tz_assert(!buf.is_null(), "BufferResource::from_one(5u) is wrongly considered a null buffer.");
		auto span = buf.data_as<const unsigned int>();
		tz_assert(span.size() == 1, "BufferResource::from_one(5u) -> data_as<uint>.size() == %zu when it should be %d", span.size(), 1);
	}
	// Check that BufferResource::from_many returns expected data size.
	{
		auto buf = tz::gl::BufferResource::from_many({1.0f, 2.0f, 3.0f});
		tz_assert(!buf.is_null(), "BufferResource::from_many({1.0f, 2.0f, 3.0f}) is wrongly considered a null buffer.");
		auto span = buf.data_as<const float>();
		std::array<float, 3> expected{1.0f, 2.0f, 3.0f};
		tz_assert(std::equal(span.begin(), span.end(), expected.begin()), "BufferResource::from_many({1.0f, 2.0f, 3.0f}) != array<float, 3>{1.0f, 2.0f, 3.0f}");
	}
}

//--------------------------------------------------------------------------------------------------
void basic_image()
{
	{
		constexpr tz::Vec2ui dims{3u, 12u};
		auto img = tz::gl::ImageResource::from_uninitialised
		({
			.format = tz::gl::ImageFormat::RGBA32,
			.dimensions = dims
		});
		tz_assert(!img.is_null(), "ImageResource::from_unitialised(...) is wrongly considered a null image resource.");
		tz_assert(img.get_dimensions() == dims, "ImageResource::from_uninitialised{..., .dimensions = {%uu, %uu}}.get_dimensions() wrongly == {%u, %u}", dims[0], dims[1], img.get_dimensions()[0], img.get_dimensions()[1]);
		const std::size_t expected = tz::gl::pixel_size_bytes(img.get_format()) * dims[0] * dims[1];
		tz_assert(img.data().size_bytes() == expected, "%ux%u image data size was expected to be %zu bytes, but instead its %zu", dims[0], dims[1], expected, img.data().size_bytes());
	}
}

//--------------------------------------------------------------------------------------------------
int main()
{
	tz::initialise
	({
		.name = "tz_resource_test",
		.flags = {tz::ApplicationFlag::HiddenWindow}
	});
	{
		null_buffer();
		null_image();
		basic_buffer();
		basic_image();
	}
	tz::terminate();
}
