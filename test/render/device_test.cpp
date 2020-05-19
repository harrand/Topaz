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

constexpr const char *vertexShaderSource = "#version 430\n"
    "void main()\n"
    "{\n"
    "}\n";
constexpr const char *fragmentShaderSource = "#version 430\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
	"	FragColor = vec4(0.0);\n"
    "}\n";

tz::test::Case broken_devices()
{
	tz::test::Case test_case("tz::render::Device Draw Tests");
    tz::render::Device nul = tz::render::Device::null_device();
    // Null device absolutely isn't ready.
    topaz_expect(test_case, !nul.ready(), "tz::render::Device's null-device should never be ready!");
    
	tz::gl::IFrame* frame = tz::core::get().window().get_frame();
    tz::render::Device frame_only{frame, nullptr, nullptr};
    // We assume that the window's frame is complete.
    topaz_assert(frame->complete(), "tz::core Window's IFrame was incomplete. This is unexpected.");
    topaz_expect(test_case, !frame_only.ready(), "An invalid tz::render::Device with only a valid IFrame is wrongly considered to be ready.");

    // A program with no shaders attached and thus unusable.
    tz::gl::ShaderProgram unusable_prog;
    topaz_assert(!unusable_prog.usable(), "Default ShaderProgram is usable?!?");

    // A valid (but useless) program.
    tz::gl::ShaderProgram prog;
    {
        tz::gl::ShaderCompiler cpl;
		tz::gl::Shader* vs = prog.emplace(tz::gl::ShaderType::Vertex);
		vs->upload_source(vertexShaderSource);
		tz::gl::Shader* fs = prog.emplace(tz::gl::ShaderType::Fragment);
		fs->upload_source(fragmentShaderSource);

		auto cpldiag_vs = cpl.compile(*vs);
		auto cpldiag_fs = cpl.compile(*fs);
		auto lnkdiag = cpl.link(prog);
        topaz_assert(cpldiag_vs.successful() && cpldiag_fs.successful() && lnkdiag.successful(), "Valid ShaderProgram components failed to compile && link. Uh oh!");
    }

    // A simple run-of-the-mill object with no data. Shouldn't pass sanity-check but that's not part of a readiness check.
    tz::gl::Object obj;

    tz::render::Device good_dev{frame, &prog, &obj};
    topaz_expect(test_case, good_dev.ready(), "Good tz::render::Device wrongly considered not to be ready!");
    tz::render::Device bad_dev{frame, &unusable_prog, &obj};
    topaz_expect(test_case, !bad_dev.ready(), "Bad tz::render::Device wrongly considered to be ready!");
    return test_case;
}

int main()
{
    tz::test::Unit device;

    // We require topaz to be initialised.
    {
        tz::core::initialise("Render Device Tests");
		device.add(broken_devices());
        tz::core::terminate();
    }
    return device.result();
}