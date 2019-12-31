//
// Created by Harrand on 30/12/2019.
//

#include "test_framework.hpp"
#include "core/core.hpp"
#include "core/tz_glad/glad_context.hpp"
#include "GLFW/glfw3.h"
#include "render/device.hpp"
#include "gl/object.hpp"
#include "gl/shader.hpp"
#include "gl/shader_compiler.hpp"
#include "gl/frame.hpp"

tz::test::Case render_example()
{
	tz::test::Case test_case("tz::render::Device Draw Tests");
	
	return test_case;
}

int main()
{
    tz::test::Unit device;

    // We require topaz to be initialised.
    {
        tz::core::initialise("Render Device Tests");
		device.add(render_example());
        tz::core::terminate();
    }
    return device.result();
}