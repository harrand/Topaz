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

constexpr const char *vtx_shader_src = R"GLSL(
	#version 430
	void main()
	{
	}
	)GLSL";
constexpr const char *frg_shader_src = R"GLSL(
	#version 430
	out vec4 FragColor;
	void main()
	{
		FragColor = vec4(0.0);
	}
	)GLSL";

tz::gl::ShaderProgram valid_program()
{
	tz::gl::ShaderProgram prog;
	{
		tz::gl::ShaderCompiler cpl;
		tz::gl::Shader* vs = prog.emplace(tz::gl::ShaderType::Vertex);
		vs->upload_source(vtx_shader_src);
		tz::gl::Shader* fs = prog.emplace(tz::gl::ShaderType::Fragment);
		fs->upload_source(frg_shader_src);

		auto cpldiag_vs = cpl.compile(*vs);
		auto cpldiag_fs = cpl.compile(*fs);
		auto lnkdiag = cpl.link(prog);
		topaz_assert(cpldiag_vs.successful() && cpldiag_fs.successful() && lnkdiag.successful(), "Valid ShaderProgram components failed to compile && link. Uh oh!");
	}
	return std::move(prog);
}

TZ_TEST_BEGIN(broken_devices)
	tz::render::Device nul = tz::render::Device::null_device();
	// Null device absolutely isn't ready.
	topaz_expect(!nul.ready(), "tz::render::Device's null-device should never be ready!");
	
	tz::gl::IFrame* frame = tz::core::get().window().get_frame();
	tz::render::Device frame_only{frame, nullptr, nullptr};
	// We assume that the window's frame is complete.
	topaz_assert(frame->complete(), "tz::core Window's IFrame was incomplete. This is unexpected.");
	topaz_expect(!frame_only.ready(), "An invalid tz::render::Device with only a valid IFrame is wrongly considered to be ready.");

	// A program with no shaders attached and thus unusable.
	tz::gl::ShaderProgram unusable_prog;
	topaz_assert(!unusable_prog.usable(), "Default ShaderProgram is usable?!?");

	// A valid (but useless) program.
	tz::gl::ShaderProgram prog = valid_program();

	// A simple run-of-the-mill object with no data. Shouldn't pass sanity-check but that's not part of a readiness check.
	tz::gl::Object obj;

	tz::render::Device good_dev{frame, &prog, &obj};
	topaz_expect(good_dev.ready(), "Good tz::render::Device wrongly considered not to be ready!");
	tz::render::Device bad_dev{frame, &unusable_prog, &obj};
	topaz_expect(!bad_dev.ready(), "Bad tz::render::Device wrongly considered to be ready!");
TZ_TEST_END

TZ_TEST_BEGIN(edit_device)
	tz::gl::ShaderProgram prog = valid_program();
	// Create dummy object with no IBO.
	tz::gl::Object dummy_object;
	
	tz::render::Device device = tz::render::Device::null_device();
	topaz_expect(!device.ready(), "Null tz::render::Device wrongly considered to be ready.");
	device.set_frame(tz::core::get().window().get_frame());
	// Program and object still invalid so this should still be unready.
	topaz_expect(!device.ready(), "Edited tz::render::Device wrongly considered to be ready (Missing program and object)");
	device.set_program(&prog);
	// Object still invalid so this should still be unready.
	topaz_expect(!device.ready(), "Edited tz::render::Device wrongly considered to be ready. (Missing object)");
	device.set_object(&dummy_object);
	// Note that no ibo handle was provided (so this would early-out if we attempt to render it). But that is irrelevant to readiness.
	topaz_expect(device.ready(), "Amended null tz::render::Device wrongly considered not to be ready.");
TZ_TEST_END

TZ_TEST_BEGIN(resource_buffers)
	tz::render::Device dev = tz::render::Device::null_device();

	tz::gl::SSBO ssbo{0};
	tz::gl::UBO ubo{1};

	topaz_expect(!dev.contains_resource_buffer(&ssbo), "Null device thinks it contains an SSBO!");
	topaz_expect(!dev.contains_resource_buffer(&ubo), "Null device thinks it contains a UBO!");
	dev.add_resource_buffer(&ssbo);
	topaz_expect(dev.contains_resource_buffer(&ssbo), "Device wrongly thinks it hasn't registered an SSBO");
	dev.add_resource_buffer(&ubo);
	topaz_expect(dev.contains_resource_buffer(&ubo), "Device wrongly thinks it hasn't registered a UBO");
	dev.remove_resource_buffer(&ssbo);
	topaz_expect(!dev.contains_resource_buffer(&ssbo), "Device thinks it contains an SSBO after it was just removed!");
	dev.remove_resource_buffer(&ubo);
	topaz_expect(!dev.contains_resource_buffer(&ubo), "Device thinks it contains a UBO after it was just removed!");
TZ_TEST_END

int main()
{
	tz::test::Unit device;

	// We require topaz to be initialised.
	{
		tz::core::initialise("Render Device Tests", tz::core::invisible_tag);
		device.add(broken_devices());
		device.add(edit_device());
		device.add(resource_buffers());
		tz::core::terminate();
	}
	return device.result();
}