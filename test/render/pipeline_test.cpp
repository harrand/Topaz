//
// Created by Harrand on 19/05/2020.
//

#include "test_framework.hpp"
#include "core/core.hpp"
#include "render/pipeline.hpp"

tz::test::Case ctor()
{
	tz::test::Case test_case("tz::render::Pipeline Ctor Tests");
	tz::render::Pipeline empty;
	topaz_expect(test_case, empty.empty(), "tz::render::Pipeline default construction is wrongly considered not to be empty.");
	return test_case;
}

tz::test::Case add_and_clear()
{
	tz::test::Case test_case("tz::render::Pipeline Add Tests");
	tz::render::Pipeline pipe;
	tz::render::Device dev = tz::render::Device::null_device();
	pipe.add(dev);
	topaz_expect(test_case, !pipe.empty() && pipe.size() == 1, "tz::render::Pipeline had unexpected size. Expected 1, got ", pipe.size());
	pipe.purge();
	topaz_expect(test_case, pipe.empty(), "tz::render::Pipeline had unexpected size. Expected 0, got ", pipe.size());
	return test_case;
}

int main()
{
    tz::test::Unit pipeline;

    // We require topaz to be initialised.
    {
        tz::core::initialise("Render Pipeline Tests");
		pipeline.add(ctor());
		pipeline.add(add_and_clear());
        tz::core::terminate();
    }
    return pipeline.result();
}