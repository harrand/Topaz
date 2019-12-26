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
    topaz_expect(test_case, !buf->is_mapped(), "tz::gl::IBuffer thinks it's mapped when it really shouldn't be...");
    topaz_expect(test_case, !buf->empty(), "tz::gl::IBuffer still thought it was empty after a resize.");
    {
        tz::mem::Block mapping = buf->map();
        topaz_expect(test_case, buf->is_mapped(), "tz::gl::IBuffer doesn't think it's mapped when it definitely should be...");
        topaz_expect(test_case, mapping.size() == (sizeof(float) * amt), "tz::gl::IBuffer mapping had unexpected size. Expected ", sizeof(float)*amt, ", but got ", mapping.size());
        topaz_expect_assert(test_case, false, "Unexpected assert invoked while testing tz::gl::Buffer Mapping. There are several possible causes -- Consider debugging.");
        buf->unmap();
    }

    constexpr float test_val = 862.123f;
    // Let's map directly to a pool and do an edit.
    {
        tz::mem::UniformPool<float> floats = buf->map_pool<float>();
        floats.set(0, test_val);
        // Should definitely have capacity of 5.
        topaz_expect(test_case, floats.capacity() == 5, "Uniform float pool had unexpected capacity. Expected ", 5, " but got ", floats.capacity());
        buf->unmap();
    }
    // First thing in the mapped block should now be a float with the value of test_val.
    // Because a UniformPool amends the type-system (and float is not a struct with const members), we can type-pun it without issue.
    // Let's re-map it and check.
    {
        tz::mem::Block blk = buf->map();
        float first = *reinterpret_cast<float*>(blk.begin);
        topaz_expect(test_case, first == (test_val), "tz::gl::Buffer UniformPool mapping did not reflect in the VRAM data store. Expected value, ", test_val, ", but got ", first);
        buf->unmap();
    }
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