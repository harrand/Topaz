#include "tz/core/tz.hpp"
#include "tz/core/assert.hpp"
#include "tz/gl/resource.hpp"

void empty_resources()
{
	using namespace tz::gl;
	BufferResource bres = BufferResource::from_one<int>(5);
	ImageResource ires = ImageResource::null();
}

void api_correctness()
{
	using namespace tz::gl;
	// Static fixed resources.
	{
		BufferResource bres0 = BufferResource::from_one<int>(0, {.access = tz::gl::ResourceAccess::StaticFixed});
		tz_assert(bres0.get_type() == ResourceType::Buffer, "BufferResource does not have ResourceType::Buffer");
		tz_assert(bres0.get_access() == ResourceAccess::StaticFixed, "StaticFixed BufferResource does not have ResourceAccess::StaticFixed");

		ImageResource ires0 = ImageResource::null();
		tz_assert(ires0.get_type() == ResourceType::Image, "ImageResource does not have ResourceType::Image");
		tz_assert(ires0.get_access() == ResourceAccess::StaticFixed, "StaticFixed ImageResource does not have ResourceAccess::StaticFixed");
	}
	// Don't specify resource access (should default to StaticFixed)
	{
		BufferResource bres0 = BufferResource::from_one<int>(0);
		tz_assert(bres0.get_type() == ResourceType::Buffer, "BufferResource does not have ResourceType::Buffer");
		tz_assert(bres0.get_access() == ResourceAccess::StaticFixed, "BufferResource that doesnt specify access does not have ResourceAccess::StaticFixed");

		ImageResource ires0 = ImageResource::null();
		tz_assert(ires0.get_type() == ResourceType::Image, "ImageResource does not have ResourceType::Image");
		tz_assert(ires0.get_access() == ResourceAccess::StaticFixed, "ImageResource that doesnt specify access does not have ResourceAccess::StaticFixed");
	}
	// DynamicFixed resources
	{
		BufferResource bres0 = BufferResource::from_one<int>(0, {.access = ResourceAccess::DynamicFixed});
		tz_assert(bres0.get_type() == ResourceType::Buffer, "BufferResource does not have ResourceType::Buffer");
		tz_assert(bres0.get_access() == ResourceAccess::DynamicFixed, "DynamicFixed BufferResource does not have ResourceAccess::DynamicFixed");

		ImageResource ires0 = ImageResource::from_uninitialised
		({
		 	.format = tz::gl::ImageFormat::R8,
			.dimensions = {1u, 1u},
			.access = tz::gl::ResourceAccess::DynamicFixed
		});
		tz_assert(ires0.get_type() == ResourceType::Image, "ImageResource does not have ResourceType::Image");
		tz_assert(ires0.get_access() == ResourceAccess::DynamicFixed, "DynamicFixed ImageResource does not have ResourceAccess::DynamicFixed");
	}
	// DynamicVariable resources
	{
		BufferResource bres0 = BufferResource::from_one<int>(0, {.access = ResourceAccess::DynamicVariable});
		tz_assert(bres0.get_type() == ResourceType::Buffer, "BufferResource does not have ResourceType::Buffer");
		tz_assert(bres0.get_access() == ResourceAccess::DynamicVariable, "DynamicVariable BufferResource does not have ResourceAccess::DynamicVariable");

		ImageResource ires0 = ImageResource::from_uninitialised
		({
		 	.format = tz::gl::ImageFormat::R8,
			.dimensions = {1u, 1u},
			.access = tz::gl::ResourceAccess::DynamicVariable
		});

		tz_assert(ires0.get_type() == ResourceType::Image, "ImageResource does not have ResourceType::Image");
		tz_assert(ires0.get_access() == ResourceAccess::DynamicVariable, "DynamicVariable ImageResource does not have ResourceAccess::DynamicVariable");
	}
}

void data_correctness()
{
	using namespace tz::gl;
	// Ensure buffer resource has expected size/data when it has one element.
	BufferResource buf = BufferResource::from_one<float>(420.69f);
	tz_assert(buf.data().size_bytes() == sizeof(float), "BufferResource (one) had unexpected size. Expected %u, got %zu", sizeof(float), buf.data().size_bytes());
	tz_assert(buf.data_as<float>().front() == 420.69f, "BufferResource (one) had invalid data. Expected float value %.2f, got %.2f", 420.69f, buf.data_as<float>().front()); // Ensure buffer resource has expected size/data when it has many elements.
	BufferResource buf0 = BufferResource::from_many
	({
		0,
		1,
		2
	});
	std::span<const int> buf0_data = buf0.data_as<const int>();
	tz_assert(buf0_data.size() == 3, "BufferResource (many) has unexpected size. Expected %u ints, got %zu ints (%u bytes, %zu bytes)", 3u, buf0_data.size(), 3u * sizeof(int), buf0_data.size_bytes());
	for(std::size_t i = 0; i < 3; i++)
	{
		tz_assert(std::cmp_equal(buf0_data[i], i), "BufferResource (many, iota ints) has invalid data. Expected %zu at %zu'th element, but it was actually %d", i, i, buf0_data[i]);
	}
}

int main()
{
	tz::initialise
	({
		.name = "tz_resource_test",
		.flags = {tz::ApplicationFlag::HiddenWindow}
	});
	{
		empty_resources();
		api_correctness();
		data_correctness();
	}
	tz::terminate();
}
