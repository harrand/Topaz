//
// Created by Harrand on 30/12/2019.
//

#include "test_framework.hpp"
#include "core/core.hpp"
#include "core/tz_glad/glad_context.hpp"
#include "gl/frame.hpp"
#include "algo/static.hpp"

TZ_TEST_BEGIN(statics)
	topaz_expect(!tz::algo::copyable<tz::gl::Frame>(), "tz::gl::Frame is copyable. This is wrong.");
	topaz_expect(tz::algo::moveable<tz::gl::Frame>(), "tz::gl::Frame is not moveable. This is wrong.");
	
	topaz_expect(!tz::algo::copyable<tz::gl::WindowFrame>(), "tz::gl::Frame is copyable. This is wrong.");
	topaz_expect(tz::algo::moveable<tz::gl::WindowFrame>(), "tz::gl::Frame is not moveable. This is wrong.");
TZ_TEST_END

TZ_TEST_BEGIN(frame_bindings)
	tz::gl::Frame f1{800, 600};
	tz::gl::Frame f2{1920, 1080};
	topaz_expect(f1 != tz::gl::bound::frame(), "tz::gl::Frame thinks it's bound before I try to bind it!");
	topaz_expect(f2 != tz::gl::bound::frame(), "tz::gl::Frame thinks it's bound before I try to bind it!");
	f1.bind();
	topaz_expect(f1 == tz::gl::bound::frame(), "tz::gl::Frame failed to notice that it had been bound!");
	topaz_expect(f2 != tz::gl::bound::frame(), "tz::gl::Frame thinks it's bound before I try to bind it!");
	f2.bind();
	topaz_expect(f1 != tz::gl::bound::frame(), "tz::gl::Frame thinks it's bound before I try to bind it!");
	topaz_expect(f2 == tz::gl::bound::frame(), "tz::gl::Frame failed to notice that it had been bound!");
	topaz_expect_assert(false, "tz::gl::Frame asserted unexpectedly!");
TZ_TEST_END

TZ_TEST_BEGIN(window_frame_bindings)
	tz::IWindow& wnd = tz::get().window();
	wnd.set_active_context();
	tz::gl::Frame f1{800, 600};
	tz::gl::Frame f2{1920, 1080};
	topaz_expect(wnd.get_frame() != nullptr, "tz::IWindow doesn't have a frame attached! That should never happen!");
	wnd.get_frame()->bind();
	topaz_expect(wnd.get_frame()->complete(), "tz::gl::IWindow's Frame was not complete.");
	topaz_expect(f1 != tz::gl::bound::frame(), "tz::gl::Frame wrongly thinks it was bound.");
	topaz_expect(f2 != tz::gl::bound::frame(), "tz::gl::Frame thinks it's bound before I try to bind it!");
	topaz_expect(wnd.get_frame()->operator==(tz::gl::bound::frame()), "tz::gl::WindowFrame doesn't think it's bound when it is!");

	/*
	topaz_expect(f1.complete(), "New tz::gl::Frame is not yet complete!");
	f1.bind();
	topaz_expect(f1 == tz::gl::bound::frame(), "tz::gl::Frame failed to notice that it had been bound!");
	topaz_expect(f2 != tz::gl::bound::frame(), "tz::gl::Frame thinks it's bound before I try to bind it!");
	topaz_expect(wnd.get_frame()->operator!=(tz::gl::bound::frame()), "tz::gl::WindowFrame thinks it's been bound when it's not!");
	*/
TZ_TEST_END

int main()
{
	tz::test::Unit frame;

	// We require topaz to be initialised.
	{
		tz::initialise("Frame Tests", tz::invisible_tag);
		frame.add(frame_bindings());

		frame.add(window_frame_bindings());
		tz::terminate();
	}
	return frame.result();
}