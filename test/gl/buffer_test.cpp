//
// Created by Harrand on 25/12/2019.
//

#include "test_framework.hpp"
#include "core/core.hpp"
#include "core/tz_glad/glad_context.hpp"
#include "gl/object.hpp"

tz::test::Case binding()
{
    tz::test::Case test_case("tz::gl::Buffer Binding Tests");
    tz::gl::Object obj;
    std::size_t idx = obj.emplace_buffer<tz::gl::BufferType::Array>();
    tz::gl::VertexBuffer* buf = obj.get<tz::gl::BufferType::Array>(idx);
    obj.bind_child(idx);
    topaz_expect(test_case, *buf == tz::gl::bound::vertex_buffer(), "tz::gl::IBuffer bind failed to reflect in global state (global state handle = ", tz::gl::bound::vertex_buffer(), ")");
    buf->unbind();
    topaz_expect(test_case, *buf != tz::gl::bound::vertex_buffer(), "tz::gl::IBuffer unbind failed to reflect in global state (global state handle = ", tz::gl::bound::vertex_buffer(), ")");
    return test_case;
}

tz::test::Case mapping()
{
    tz::test::Case test_case("tz::gl::Buffer Mapping Tests");
    tz::gl::Object obj;
    std::size_t idx = obj.emplace_buffer<tz::gl::BufferType::Array>();
    tz::gl::IBuffer* buf = obj[idx];
    topaz_expect(test_case, !buf->valid(), "tz::gl::Buffer was wrongly considered valid before its first bind.");
    buf->bind();
    topaz_expect(test_case, buf->valid(), "tz::gl::Buffer was wrongly considered invalid after its first binding");
    topaz_expect(test_case, glGetError() == 0, "glGetError() displayed an error!");
    // Now we have a valid buffer. Bind things properly first.
    obj.bind_child(idx);
    topaz_expect(test_case, buf != nullptr, "tz::gl::Object[", idx, "] gave nullptr. Something is horribly wrong.");
    topaz_expect(test_case, buf->empty(), "tz::gl::IBuffer constructed in object is not empty!");
    topaz_expect_assert(test_case, false, "tz::gl::IBuffer asserted at the wrong time (Probably from Buffer<T>::size())...");
    constexpr std::size_t amt = 5;

    // Now lets allocate some data in it.
    buf->resize(sizeof(float) * amt);
    topaz_expect(test_case, !buf->empty(), "tz::gl::IBuffer still thought it was empty after a resize.");
    void* mapping = buf->map();
    topaz_expect_assert(test_case, false, "Unexpected assert invoked while testing tz::gl::Buffer Mapping. There are several possible causes -- Consider debugging.");
    buf->unmap();
    //tz::mem::UniformPool<float> pool = buf->map<float>(amt);
    // Now we have a pool of size 5. Ensure nothing has asserted thus far before continuing.
    //topaz_expect(test_case, pool.capacity() == amt, "tz::gl::IBuffer::map(): Produced malformed tz::mem::UniformPool<T>. Expected capacity of ", amt, ", but got ", pool.capacity());
    return test_case;
}

int main()
{
    tz::test::Unit buffer;

    // We require topaz to be initialised.
    {
        tz::core::initialise("Buffer Tests");

        buffer.add(binding());
        buffer.add(mapping());

        tz::core::terminate();
    }
    return buffer.result();
}