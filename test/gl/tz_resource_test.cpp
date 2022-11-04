#include "tz/core/tz.hpp"
#include "tz/core/assert.hpp"
#include "tz/core/report.hpp"
#include "tz/gl/resource.hpp"

//--------------------------------------------------------------------------------------------------
void null_buffer()
{
	tz_assert(tz::gl::BufferResource::null().is_null(), "Null buffer is not a null buffer");
	tz_assert(tz::gl::BufferResource::null().get_type() == tz::gl::ResourceType::Buffer, "Null buffer is not a buffer at all!");
}

//--------------------------------------------------------------------------------------------------
void null_image()
{
	tz_assert(tz::gl::ImageResource::null().is_null(), "Null image is not a null image");
	tz_assert(tz::gl::ImageResource::null().get_type() == tz::gl::ResourceType::Image, "Null image is not an image at all!");
}


//--------------------------------------------------------------------------------------------------
void basic_buffer()
{
	// Check that BufferResource::from_one returns expected data size.
	{
		auto buf = tz::gl::BufferResource::from_one(5u);
		tz_assert(!buf.is_null(), "BufferResource::from_one(5u) is wrongly considered a null buffer.");
		tz_assert(buf.get_type() == tz::gl::ResourceType::Buffer, "BufferResource::from_one(5u) is not ResourceType::Buffer!");
		auto span = buf.data_as<const unsigned int>();
		tz_assert(span.size() == 1, "BufferResource::from_one(5u) -> data_as<uint>.size() == %zu when it should be %d", span.size(), 1);
	}
	// Check that BufferResource::from_many returns expected data size.
	{
		auto buf = tz::gl::BufferResource::from_many({1.0f, 2.0f, 3.0f});
		tz_assert(buf.get_type() == tz::gl::ResourceType::Buffer, "BufferResource::from_many({1.0f, 2.0f, 3.0f}) is not ResourceType::Buffer!");
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
		tz_assert(img.get_type() == tz::gl::ResourceType::Image, "ImageResource::from_uninitialised(...) is not ResourceType::Image!");
		tz_assert(img.get_dimensions() == dims, "ImageResource::from_uninitialised{..., .dimensions = {%uu, %uu}}.get_dimensions() wrongly == {%u, %u}", dims[0], dims[1], img.get_dimensions()[0], img.get_dimensions()[1]);
		const std::size_t expected = tz::gl::pixel_size_bytes(img.get_format()) * dims[0] * dims[1];
		tz_assert(img.data().size_bytes() == expected, "%ux%u image data size was expected to be %zu bytes, but instead its %zu", dims[0], dims[1], expected, img.data().size_bytes());
	}
}

//--------------------------------------------------------------------------------------------------
void default_arguments_buffer()
{
	auto nullbuf = tz::gl::BufferResource::null();
	tz_assert(nullbuf.get_access() == tz::gl::ResourceAccess::Static, "Null Buffer must be ResourceAccess::Static, but it is not.");
	tz_assert(nullbuf.get_flags().empty(), "Null Buffer must not have any flags");

	{
		auto buf = tz::gl::BufferResource::from_one(1);
		tz_assert(buf.get_access() == tz::gl::ResourceAccess::Static, "Defaulted Buffer must be ResourceAccess::Static, but it is not.");
		tz_assert(buf.get_flags().empty(), "Defaulted Buffer must not have any flags");
	}
	{
		auto buf = tz::gl::BufferResource::from_many({1, 2, 3});
		tz_assert(buf.get_access() == tz::gl::ResourceAccess::Static, "Defaulted Buffer must be ResourceAccess::Static, but it is not.");
		tz_assert(buf.get_flags().empty(), "Defaulted Buffer must not have any flags");
	}
}

//--------------------------------------------------------------------------------------------------
void default_arguments_image()
{
	auto nullimg = tz::gl::ImageResource::null();
	tz_assert(nullimg.get_access() == tz::gl::ResourceAccess::Static, "Null Image must be ResourceAccess::Static, but it is not.");
	tz_assert(nullimg.get_flags().empty(), "Null Image must not have any flags");

	{
		auto img = tz::gl::ImageResource::from_uninitialised({.format = tz::gl::ImageFormat::RGBA32, .dimensions = {1u, 1u}});
		tz_assert(img.get_access() == tz::gl::ResourceAccess::Static, "Defaulted Image must be ResourceAccess::Static, but it is not.");
		tz_assert(img.get_flags().empty(), "Defaulted Image must not have any flags");
	}
	{
		auto img = tz::gl::ImageResource::from_memory(
		{
			std::byte{1},
			std::byte{2},
			std::byte{3}
		}, {.format = tz::gl::ImageFormat::RGBA32, .dimensions = {1u, 1u}});
		tz_assert(img.get_access() == tz::gl::ResourceAccess::Static, "Defaulted Image must be ResourceAccess::Static, but it is not.");
		tz_assert(img.get_flags().empty(), "Defaulted Image must not have any flags");
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

		default_arguments_buffer();
		default_arguments_image();
	}
	tz::terminate();
}
