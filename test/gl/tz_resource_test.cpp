#include "tz/tz.hpp"
#include "tz/core/debug.hpp"
#include "tz/core/debug.hpp"
#include "tz/gl/resource.hpp"

//--------------------------------------------------------------------------------------------------
void null_buffer()
{
	tz::assert(tz::gl::buffer_resource::null().is_null(), "Null buffer is not a null buffer");
	tz::assert(tz::gl::buffer_resource::null().get_type() == tz::gl::resource_type::buffer, "Null buffer is not a buffer at all!");
}

//--------------------------------------------------------------------------------------------------
void null_image()
{
	tz::assert(tz::gl::image_resource::null().is_null(), "Null image is not a null image");
	tz::assert(tz::gl::image_resource::null().get_type() == tz::gl::resource_type::image, "Null image is not an image at all!");
}


//--------------------------------------------------------------------------------------------------
void basic_buffer()
{
	// Check that buffer_resource::from_one returns expected data size.
	{
		auto buf = tz::gl::buffer_resource::from_one(5u);
		tz::assert(!buf.is_null(), "buffer_resource::from_one(5u) is wrongly considered a null buffer.");
		tz::assert(buf.get_type() == tz::gl::resource_type::buffer, "buffer_resource::from_one(5u) is not resource_type::buffer!");
		auto span = buf.data_as<const unsigned int>();
		tz::assert(span.size() == 1, "buffer_resource::from_one(5u) -> data_as<uint>.size() == %zu when it should be %d", span.size(), 1);
	}
	// Check that buffer_resource::from_many returns expected data size.
	{
		auto buf = tz::gl::buffer_resource::from_many({1.0f, 2.0f, 3.0f});
		tz::assert(buf.get_type() == tz::gl::resource_type::buffer, "buffer_resource::from_many({1.0f, 2.0f, 3.0f}) is not resource_type::buffer!");
		tz::assert(!buf.is_null(), "buffer_resource::from_many({1.0f, 2.0f, 3.0f}) is wrongly considered a null buffer.");
		auto span = buf.data_as<const float>();
		std::array<float, 3> expected{1.0f, 2.0f, 3.0f};
		tz::assert(std::equal(span.begin(), span.end(), expected.begin()), "buffer_resource::from_many({1.0f, 2.0f, 3.0f}) != array<float, 3>{1.0f, 2.0f, 3.0f}");
	}
}

//--------------------------------------------------------------------------------------------------
void basic_image()
{
	{
		constexpr tz::vec2ui dims{3u, 12u};
		auto img = tz::gl::image_resource::from_uninitialised
		({
			.format = tz::gl::image_format::RGBA32,
			.dimensions = dims
		});
		tz::assert(!img.is_null(), "image_resource::from_unitialised(...) is wrongly considered a null image resource.");
		tz::assert(img.get_type() == tz::gl::resource_type::image, "image_resource::from_uninitialised(...) is not resource_type::image!");
		tz::assert(img.get_dimensions() == dims, "image_resource::from_uninitialised{..., .dimensions = {%uu, %uu}}.get_dimensions() wrongly == {%u, %u}", dims[0], dims[1], img.get_dimensions()[0], img.get_dimensions()[1]);
		const std::size_t expected = tz::gl::pixel_size_bytes(img.get_format()) * dims[0] * dims[1];
		tz::assert(img.data().size_bytes() == expected, "%ux%u image data size was expected to be %zu bytes, but instead its %zu", dims[0], dims[1], expected, img.data().size_bytes());
	}
}

//--------------------------------------------------------------------------------------------------
void default_arguments_buffer()
{
	auto nullbuf = tz::gl::buffer_resource::null();
	tz::assert(nullbuf.get_access() == tz::gl::resource_access::static_access, "Null Buffer must be resource_access::static_access, but it is not.");
	tz::assert(nullbuf.get_flags().empty(), "Null Buffer must not have any flags");

	{
		auto buf = tz::gl::buffer_resource::from_one(1);
		tz::assert(buf.get_access() == tz::gl::resource_access::static_access, "Defaulted Buffer must be resource_access::static_access, but it is not.");
		tz::assert(buf.get_flags().empty(), "Defaulted Buffer must not have any flags");
	}
	{
		auto buf = tz::gl::buffer_resource::from_many({1, 2, 3});
		tz::assert(buf.get_access() == tz::gl::resource_access::static_access, "Defaulted Buffer must be resource_access::static_access, but it is not.");
		tz::assert(buf.get_flags().empty(), "Defaulted Buffer must not have any flags");
	}
}

//--------------------------------------------------------------------------------------------------
void default_arguments_image()
{
	auto nullimg = tz::gl::image_resource::null();
	tz::assert(nullimg.get_access() == tz::gl::resource_access::static_access, "Null Image must be resource_access::static_access, but it is not.");
	tz::assert(nullimg.get_flags().empty(), "Null Image must not have any flags");

	{
		auto img = tz::gl::image_resource::from_uninitialised({.format = tz::gl::image_format::RGBA32, .dimensions = {1u, 1u}});
		tz::assert(img.get_access() == tz::gl::resource_access::static_access, "Defaulted Image must be resource_access::static_access, but it is not.");
		tz::assert(img.get_flags().empty(), "Defaulted Image must not have any flags");
	}
	{
		auto img = tz::gl::image_resource::from_memory(
		{
			std::byte{1},
			std::byte{2},
			std::byte{3}
		}, {.format = tz::gl::image_format::RGBA32, .dimensions = {1u, 1u}});
		tz::assert(img.get_access() == tz::gl::resource_access::static_access, "Defaulted Image must be resource_access::static_access, but it is not.");
		tz::assert(img.get_flags().empty(), "Defaulted Image must not have any flags");
	}
}

//--------------------------------------------------------------------------------------------------
int main()
{
	tz::initialise
	({
		.name = "tz_resource_test",
		.flags = {tz::application_flag::window_hidden}
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
