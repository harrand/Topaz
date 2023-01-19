#include "tz/core/tz.hpp"
#include "hdk/debug.hpp"
#include "hdk/debug.hpp"
#include "tz/gl/resource.hpp"

//--------------------------------------------------------------------------------------------------
void null_buffer()
{
	hdk::assert(tz::gl::BufferResource::null().is_null(), "Null buffer is not a null buffer");
	hdk::assert(tz::gl::BufferResource::null().get_type() == tz::gl::resource_type::buffer, "Null buffer is not a buffer at all!");
}

//--------------------------------------------------------------------------------------------------
void null_image()
{
	hdk::assert(tz::gl::ImageResource::null().is_null(), "Null image is not a null image");
	hdk::assert(tz::gl::ImageResource::null().get_type() == tz::gl::resource_type::image, "Null image is not an image at all!");
}


//--------------------------------------------------------------------------------------------------
void basic_buffer()
{
	// Check that BufferResource::from_one returns expected data size.
	{
		auto buf = tz::gl::BufferResource::from_one(5u);
		hdk::assert(!buf.is_null(), "BufferResource::from_one(5u) is wrongly considered a null buffer.");
		hdk::assert(buf.get_type() == tz::gl::resource_type::buffer, "BufferResource::from_one(5u) is not resource_type::buffer!");
		auto span = buf.data_as<const unsigned int>();
		hdk::assert(span.size() == 1, "BufferResource::from_one(5u) -> data_as<uint>.size() == %zu when it should be %d", span.size(), 1);
	}
	// Check that BufferResource::from_many returns expected data size.
	{
		auto buf = tz::gl::BufferResource::from_many({1.0f, 2.0f, 3.0f});
		hdk::assert(buf.get_type() == tz::gl::resource_type::buffer, "BufferResource::from_many({1.0f, 2.0f, 3.0f}) is not resource_type::buffer!");
		hdk::assert(!buf.is_null(), "BufferResource::from_many({1.0f, 2.0f, 3.0f}) is wrongly considered a null buffer.");
		auto span = buf.data_as<const float>();
		std::array<float, 3> expected{1.0f, 2.0f, 3.0f};
		hdk::assert(std::equal(span.begin(), span.end(), expected.begin()), "BufferResource::from_many({1.0f, 2.0f, 3.0f}) != array<float, 3>{1.0f, 2.0f, 3.0f}");
	}
}

//--------------------------------------------------------------------------------------------------
void basic_image()
{
	{
		constexpr hdk::vec2ui dims{3u, 12u};
		auto img = tz::gl::ImageResource::from_uninitialised
		({
			.format = tz::gl::image_format::RGBA32,
			.dimensions = dims
		});
		hdk::assert(!img.is_null(), "ImageResource::from_unitialised(...) is wrongly considered a null image resource.");
		hdk::assert(img.get_type() == tz::gl::resource_type::image, "ImageResource::from_uninitialised(...) is not resource_type::image!");
		hdk::assert(img.get_dimensions() == dims, "ImageResource::from_uninitialised{..., .dimensions = {%uu, %uu}}.get_dimensions() wrongly == {%u, %u}", dims[0], dims[1], img.get_dimensions()[0], img.get_dimensions()[1]);
		const std::size_t expected = tz::gl::pixel_size_bytes(img.get_format()) * dims[0] * dims[1];
		hdk::assert(img.data().size_bytes() == expected, "%ux%u image data size was expected to be %zu bytes, but instead its %zu", dims[0], dims[1], expected, img.data().size_bytes());
	}
}

//--------------------------------------------------------------------------------------------------
void default_arguments_buffer()
{
	auto nullbuf = tz::gl::BufferResource::null();
	hdk::assert(nullbuf.get_access() == tz::gl::resource_access::static_fixed, "Null Buffer must be resource_access::static_fixed, but it is not.");
	hdk::assert(nullbuf.get_flags().empty(), "Null Buffer must not have any flags");

	{
		auto buf = tz::gl::BufferResource::from_one(1);
		hdk::assert(buf.get_access() == tz::gl::resource_access::static_fixed, "Defaulted Buffer must be resource_access::static_fixed, but it is not.");
		hdk::assert(buf.get_flags().empty(), "Defaulted Buffer must not have any flags");
	}
	{
		auto buf = tz::gl::BufferResource::from_many({1, 2, 3});
		hdk::assert(buf.get_access() == tz::gl::resource_access::static_fixed, "Defaulted Buffer must be resource_access::static_fixed, but it is not.");
		hdk::assert(buf.get_flags().empty(), "Defaulted Buffer must not have any flags");
	}
}

//--------------------------------------------------------------------------------------------------
void default_arguments_image()
{
	auto nullimg = tz::gl::ImageResource::null();
	hdk::assert(nullimg.get_access() == tz::gl::resource_access::static_fixed, "Null Image must be resource_access::static_fixed, but it is not.");
	hdk::assert(nullimg.get_flags().empty(), "Null Image must not have any flags");

	{
		auto img = tz::gl::ImageResource::from_uninitialised({.format = tz::gl::image_format::RGBA32, .dimensions = {1u, 1u}});
		hdk::assert(img.get_access() == tz::gl::resource_access::static_fixed, "Defaulted Image must be resource_access::static_fixed, but it is not.");
		hdk::assert(img.get_flags().empty(), "Defaulted Image must not have any flags");
	}
	{
		auto img = tz::gl::ImageResource::from_memory(
		{
			std::byte{1},
			std::byte{2},
			std::byte{3}
		}, {.format = tz::gl::image_format::RGBA32, .dimensions = {1u, 1u}});
		hdk::assert(img.get_access() == tz::gl::resource_access::static_fixed, "Defaulted Image must be resource_access::static_fixed, but it is not.");
		hdk::assert(img.get_flags().empty(), "Defaulted Image must not have any flags");
	}
}

//--------------------------------------------------------------------------------------------------
int main()
{
	tz::initialise
	({
		.name = "tz_resource_test",
		.flags = {tz::application_flag::HiddenWindow}
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
