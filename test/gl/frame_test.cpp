//
// Created by Harrand on 25/12/2019.
//

#include "test_framework.hpp"
#include "core/core.hpp"
#include "core/tz_glad/glad_context.hpp"
#include "gl/frame.hpp"

tz::test::Case frame_bindings()
{
	tz::test::Case test_case("tz::gl::Frame Binding Tests");
	tz::gl::Frame f1{800, 600};
	tz::gl::Frame f2{1920, 1080};
	topaz_expect(test_case, f1 != tz::gl::bound::frame(), "tz::gl::Frame thinks it's bound before I try to bind it!");
	topaz_expect(test_case, f2 != tz::gl::bound::frame(), "tz::gl::Frame thinks it's bound before I try to bind it!");
	f1.bind();
	topaz_expect(test_case, f1 == tz::gl::bound::frame(), "tz::gl::Frame failed to notice that it had been bound!");
	topaz_expect(test_case, f2 != tz::gl::bound::frame(), "tz::gl::Frame thinks it's bound before I try to bind it!");
	f2.bind();
	topaz_expect(test_case, f1 != tz::gl::bound::frame(), "tz::gl::Frame thinks it's bound before I try to bind it!");
	topaz_expect(test_case, f2 == tz::gl::bound::frame(), "tz::gl::Frame failed to notice that it had been bound!");
	topaz_expect_assert(test_case, false, "tz::gl::Frame asserted unexpectedly!");

	return test_case;
}

tz::test::Case window_frame_bindings()
{
	tz::test::Case test_case("tz::gl::Frame + Window Binding Tests");
	tz::core::IWindow& wnd = tz::core::get().window();
	wnd.set_active_context();
	tz::gl::Frame f1{800, 600};
	tz::gl::Frame f2{1920, 1080};
	topaz_expect(test_case, wnd.get_frame() != nullptr, "tz::core::IWindow doesn't have a frame attached! That should never happen!");
	wnd.get_frame()->bind();
	topaz_expect(test_case, f1 != tz::gl::bound::frame(), "tz::gl::Frame wrongly thinks it was bound.");
	topaz_expect(test_case, f2 != tz::gl::bound::frame(), "tz::gl::Frame thinks it's bound before I try to bind it!");
	topaz_expect(test_case, wnd.get_frame()->operator==(tz::gl::bound::frame()), "tz::gl::WindowFrame doesn't think it's bound when it is!");

	f1.bind();
	topaz_expect(test_case, f1 == tz::gl::bound::frame(), "tz::gl::Frame failed to notice that it had been bound!");
	topaz_expect(test_case, f2 != tz::gl::bound::frame(), "tz::gl::Frame thinks it's bound before I try to bind it!");
	topaz_expect(test_case, wnd.get_frame()->operator!=(tz::gl::bound::frame()), "tz::gl::WindowFrame thinks it's been bound when it's not!");
	return test_case;
}

int main()
{
    tz::test::Unit frame;

    // We require topaz to be initialised.
    {
        tz::core::initialise("Frame Tests");
		frame.add(frame_bindings());

		frame.add(window_frame_bindings());
        tz::core::terminate();
    }
    return frame.result();
}