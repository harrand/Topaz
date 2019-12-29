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
    tz::gl::Object o;
    auto make_child = [&o](){return o.emplace_buffer<tz::gl::BufferType::Array>();};
    topaz_expect(test_case, o.size() == 0, "tz::gl::Object without children failed to have zero size. Size: ", o.size());
    std::size_t a = make_child();
    topaz_expect(test_case, o.size() == 1, "tz::gl::Object failed to increment its size after adding a child. Expected ", 1, ", got ", o.size());
    topaz_expect(test_case, o[a] != nullptr, "tz::gl::Object failed to resolve a valid child index. Index was ", a, " but returned nullptr.");
    return test_case;
}

tz::test::Case erase()
{
    tz::test::Case test_case("tz::gl::Object Erase Tests");
    tz::gl::Object o;
    auto make_child = [&o](){return o.emplace_buffer<tz::gl::BufferType::Array>();};
    auto a = make_child();
    auto b = make_child();
    auto c = make_child();
    topaz_expect(test_case, o.size() == 3, "tz::gl::Object failed to retrieve correct size. Expected ", 3, ", got ", o.size());
    topaz_expect(test_case, o.element_size() == 3, "tz::gl::Object failed to retrieve correct element_size. Expected ", 3, ", got ", o.size());
    topaz_expect(test_case, o[c] != nullptr, "tz::gl::Object failed to emplace buffer correctly. Buffer at index ", c, ", was unexpectedly nullptr.");
    // Firstly erase c.
    o.erase(c);
    // The size should be unchanged but the element_size should have gone down by one.
    topaz_expect(test_case, o.size() == 3, "tz::gl::Object failed to retrieve correct size. Expected ", 3, ", got ", o.size());
    topaz_expect(test_case, o.element_size() == 2, "tz::gl::Object failed to retrieve correct element_size. Expected ", 2, ", got ", o.size());
    // So there must still exist an entry at 'c'. Let's make sure it equates to nullptr.
    topaz_expect(test_case, o[c] == nullptr, "tz::gl::Object erasure at element ", c, " failed to yield nullptr afterwards.");
    return test_case;
}

tz::test::Case release()
{
    tz::test::Case test_case("tz::gl::Object Release Tests");
    tz::gl::Object o;
    auto make_child = [&o](){return o.emplace_buffer<tz::gl::BufferType::Array>();};
    auto a = make_child();
    auto b = make_child();
    auto c = make_child();
    topaz_expect(test_case, o.size() == 3, "tz::gl::Object failed to retrieve correct size. Expected ", 3, ", got ", o.size());
    topaz_expect(test_case, o.element_size() == 3, "tz::gl::Object failed to retrieve correct element_size. Expected ", 3, ", got ", o.size());
    topaz_expect(test_case, o[c] != nullptr, "tz::gl::Object failed to emplace buffer correctly. Buffer at index ", c, ", was unexpectedly nullptr.");
    // Make a second Object!
    tz::gl::Object o2;
    // Let's release a buffer from o and chuck it into o2.
    std::size_t o2_c;
    {
        auto released = o.release(c);
        topaz_expect(test_case, released != nullptr, "tz::gl::Object release invocation at a valid index ", c, " returned a handle which equated to nullptr!");
        // Chuck it into o2.
        o2_c = o2.add_buffer(std::move(released));
    }
    topaz_expect(test_case, o2.size() == o2.element_size() && o2.size() == 1, "tz::gl::Object add_buffer on a released handle did not amend its sizes correctly. Expected size ", 1, " and element_size ", 1, ", but got ", o2.size(), " and ", o2.element_size());
    topaz_expect(test_case, o2[o2_c] != nullptr, "tz::gl::Object add_buffer on a released handle did not seem to work properly -- The resultant buffer at index ", o2_c, " was nullptr.");
    return test_case;
}

tz::test::Case set()
{
    tz::test::Case test_case("tz::gl::Object Set Tests");
    tz::gl::Object o;
    auto make_child = [&o](){return o.emplace_buffer<tz::gl::BufferType::Array>();};
    auto a = make_child();
    auto b = make_child();
    auto c = make_child();
    // Will replace with a new buffer-type.
    o.set<tz::gl::BufferType::Array>(c);
    // We can't do much here but assert that the damn thing hasn't become nullptr.
    topaz_expect(test_case, o[c] != nullptr, "tz::gl::Object set invocation seems to have damaged the Buffer at index ", c);
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
        object.add(erase());
        object.add(release());
        object.add(set());

        tz::core::terminate();
    }
    return object.result();
}