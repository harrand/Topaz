//
// Created by Harrand on 25/12/2019.
//

#include "test_framework.hpp"
#include "core/core.hpp"
#include "core/tz_glad/glad_context.hpp"
#include "gl/buffer.hpp"

tz::test::Case binding()
{
    tz::test::Case test_case("tz::gl Buffer Binding Tests");

    tz::gl::VertexBuffer buf;
    buf.bind();
    topaz_expect(test_case, buf == tz::gl::bound::vertex_buffer(), "tz::gl::IBuffer bind failed to reflect in global state (global state handle = ", tz::gl::bound::vertex_buffer(), ")");

    return test_case;
}

int main()
{
    tz::test::Unit buffer;

    // We require topaz to be initialised.
    {
        tz::core::initialise("Buffer Tests");

        buffer.add(binding());

        tz::core::terminate();
    }
    return buffer.result();
}