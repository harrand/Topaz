//
// Created by Harrand on 19/05/2020.
//

#include "test_framework.hpp"
#include "core/tz.hpp"
#include "render/pipeline.hpp"

TZ_TEST_BEGIN(ctor)
	tz::render::Pipeline empty;
	topaz_expect(empty.empty(), "tz::render::Pipeline default construction is wrongly considered not to be empty.");
TZ_TEST_END

TZ_TEST_BEGIN(add_and_clear)
	tz::render::Pipeline pipe;
	tz::render::Device dev = tz::render::Device::null_device();
	pipe.add(dev);
	topaz_expect(!pipe.empty() && pipe.size() == 1, "tz::render::Pipeline had unexpected size. Expected 1, got ", pipe.size());
	pipe.purge();
	topaz_expect(pipe.empty(), "tz::render::Pipeline had unexpected size. Expected 0, got ", pipe.size());
TZ_TEST_END

int main()
{
	tz::test::Unit pipeline;

	// We require topaz to be initialised.
	{
		tz::initialise("Render Pipeline Tests", tz::invisible_tag);
		pipeline.add(ctor());
		pipeline.add(add_and_clear());
		tz::terminate();
	}
	return pipeline.result();
}