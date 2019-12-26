//
// Created by Harrand on 25/12/2019.
//

#include "test_framework.hpp"
#include "core/core.hpp"
#include "core/tz_glad/glad_context.hpp"
#include "gl/object.hpp"

tz::test::Case binding()
{
    tz::test::Case test_case("tz::gl::Object Binding Tests");

    // Make an object and ensure it binds properly.
    tz::gl::Object obj;
    obj.bind();
    topaz_expect(test_case, obj == tz::gl::bound::vao(), "tz::gl::Object bind failed to reflect in global state (global state handle = ", tz::gl::bound::vao(), ")");
    // Ensure unbinding it does something.
    obj.unbind();
    topaz_expect(test_case, obj != tz::gl::bound::vao(), "tz::gl::Object unbind failed to reflect in global state (global state handle = ", tz::gl::bound::vao(), ")");

    // Try another object (check for zero-error)
    tz::gl::Object obj2;
    obj2.bind();
    topaz_expect(test_case, obj2 == tz::gl::bound::vao(), "tz::gl::Object bind failed to reflect in global state (global state handle = ", tz::gl::bound::vao(), ")");

    // Make sure the objects are unique!
    topaz_expect(test_case, obj != obj2, "Separate tz::gl::Object wrongly equate to true");
    return test_case;
}

tz::test::Case children()
{
    tz::test::Case test_case("tz::gl::Object Child Tests");

    return test_case;
}

int main()
{
    tz::test::Unit object;

    // We require topaz to be initialised.
    {
        tz::core::initialise("Object Tests");

        object.add(binding());
        object.add(children());

        tz::core::terminate();
    }
    return object.result();
}