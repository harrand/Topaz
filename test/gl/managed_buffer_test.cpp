//
// Created by Harrand on 22/01/2020.
//

#include "test_framework.hpp"
#include "core/core.hpp"
#include "core/tz_glad/glad_context.hpp"
#include "gl/object.hpp"
#include "gl/mesh.hpp"

tz::test::Case regions()
{
    tz::test::Case test_case("tz::gl::ManagedTerminalBuffer<T> Region Tests");
    tz::gl::Object o;

    std::size_t mvbo_id = o.emplace_managed_terminal_buffer<tz::gl::BufferType::Array>(sizeof(float) * 3);
    tz::gl::ManagedTVBO& mvbo = *o.get_managed_terminal<tz::gl::BufferType::Array>(mvbo_id);

    // is now terminal so we just need to map and we can play with some regions!
    tz::mem::UniformPool<float> floats = mvbo.map_pool<float>();

    floats.set(0, 5.0f);
    floats.set(1, 10.0f);
    floats.set(2, 15.0f);
    // essentially stores [5, 10, 15] to regions [x, y, z]
    mvbo.region(0, sizeof(float), "x");
    mvbo.region(sizeof(float), sizeof(float), "y");
    mvbo.region(sizeof(float) * 2, sizeof(float), "z");

    auto get_expected = [&mvbo](const std::string& name)->float
    {
        return *(mvbo[name].block.get<float>(0));
    };
    float ex_x = get_expected("x");
    float ex_y = get_expected("y");
    float ex_z = get_expected("z");
    topaz_expect(test_case, ex_x == 5.0f, "tz::gl::ManagedTerminalBuffer<T> region yielded incorrect data. Expected ", 5.0f, ", but got ", ex_x);
    topaz_expect(test_case, ex_y == 10.0f, "tz::gl::ManagedTerminalBuffer<T> region yielded incorrect data. Expected ", 10.0f, ", but got ", ex_y);
    topaz_expect(test_case, ex_z == 15.0f, "tz::gl::ManagedTerminalBuffer<T> region yielded incorrect data. Expected ", 15.0f, ", but got ", ex_z);

    return test_case;
}

tz::test::Case mock_mesh()
{
    tz::test::Case test_case("tz::gl::ManagedTerminalBuffer<T> Region Mesh Mocking Tests");
    tz::gl::Object o;

    std::size_t mvbo_id = o.emplace_managed_terminal_buffer<tz::gl::BufferType::Array>(tz::gl::deferred_terminal_tag{});
    tz::gl::ManagedTVBO& mvbo = *o.get_managed_terminal<tz::gl::BufferType::Array>(mvbo_id);

    tz::gl::MeshData triangle;
    {
        // positions, texcoords, normals, tangents, bitangents
        // note: tangents and bitangents are undefined here.
        const tz::gl::Vertex vertices[] = {
            {{{-0.5f, -0.5f, 0.0f}}, {{0.0f, 0.0f}}, {{0.0f, 0.0f, -1.0f}}, {}, {}},
            {{{0.5f, -0.5f, -0.0f}}, {{1.0f, 0.0f}}, {{0.0f, 0.0f, -1.0f}}, {}, {}},
            {{{0.0f, 0.5f, 0.0f}}, {{0.5f, 1.0f}}, {{0.0f, 0.0f, -1.0f}}, {}, {}},
        };

        for(std::size_t i = 0; i < 3; i++)
            triangle.vertices.push_back(vertices[i]);
    }
    tz::gl::MeshData quad;
    {
        // positions, texcoords, normals, tangents, bitangents
        // note: tangents and bitangents are undefined here.
        const tz::gl::Vertex vertices[] = {
            {{{-0.5f, -0.5f, 0.0f}}, {{0.0f, 0.0f}}, {{0.0f, 0.0f, -1.0f}}, {}, {}},
            {{{0.5f, -0.5f, -0.0f}}, {{1.0f, 0.0f}}, {{0.0f, 0.0f, -1.0f}}, {}, {}},
            {{{0.5f, 0.5f, 0.0f}}, {{1.0f, 1.0f}}, {{0.0f, 0.0f, -1.0f}}, {}, {}},
            {{{-0.5f, 0.5f, 0.0f}}, {{0.0f, 1.0f}}, {{0.0f, 0.0f, -1.0f}}, {}, {}},
        };

        for(std::size_t i = 0; i < 4; i++)
            quad.vertices.push_back(vertices[i]);
    }

    const std::size_t total_vertices = quad.vertices.size() + triangle.vertices.size();
    mvbo.resize(sizeof(tz::gl::Vertex) * total_vertices);
    // Fill the vbo with packed vertex data and create sane regions.
    mvbo.send(triangle.vertices.data());
    // send the quad vertices at an offset.
    std::size_t quad_offset = sizeof(tz::gl::Vertex) * triangle.vertices.size();
    const std::size_t quad_size = sizeof(tz::gl::Vertex) * quad.vertices.size();
    tz::mem::Block quad_data{quad.vertices.data(), quad_size};
    mvbo.send(quad_offset, quad_data);
    // now make it terminal and map it.
    mvbo.make_terminal();
    mvbo.map();
    // we can safely mark the regions now.
    mvbo.region(0, quad_offset, "triangle data");
    mvbo.region(quad_offset, quad_size, "quad data");

    tz::mem::Block triangle_region = mvbo["triangle data"].block;
    tz::mem::Block quad_region = mvbo["quad data"].block;
    const bool expected_triangle_size = quad_size == quad_region.size();
    const bool expected_quad_size = quad_offset == triangle_region.size();
    topaz_expect(test_case, expected_quad_size, "tz::gl::ManagedBlock<T> yielded unexpected region size of ", quad_region.size(), ", expected ", quad_size);
    topaz_expect(test_case, expected_triangle_size, "tz::gl::ManagedBlock<T> yielded unexpected region size of ", triangle_region.size(), ", expected ", quad_offset);

    return test_case;
}

tz::test::Case defragmentation()
{
    tz::test::Case test_case("tz::gl::ManagedTerminalBuffer<T> Region Defragmentation Tests");
    tz::gl::Object o;

    std::size_t mvbo_id = o.emplace_managed_terminal_buffer<tz::gl::BufferType::Array>(sizeof(int) * 12);
    tz::gl::ManagedTVBO& mvbo = *o.get_managed_terminal<tz::gl::BufferType::Array>(mvbo_id);
    // 12 ints
    // i want to organise these ints.
    topaz_expect(test_case, mvbo.regions_usage() == 0, "tz::gl::ManagedTerminalBuffer<T> Thinks it has non-zero regions usage when there are no regions!");
    
    tz::mem::UniformPool<int> ints = mvbo.map_pool<int>();
    // ok sweet.
    // i'm going to chuck some primes in here!
    ints.set(0, 3);
    ints.set(1, 5);
    ints.set(2, 7);
    ints.set(3, 11);
    // mark this region...
    mvbo.region(0, sizeof(int) * 4, "primes");
    // time for some evens!
    ints.set(4, 2);
    ints.set(5, 4);
    ints.set(6, 6);
    ints.set(7, 8);
    ints.set(8, 10);
    mvbo.region(sizeof(int) * 4, sizeof(int) * 5, "evens");
    // now some negatives!
    ints.set(9, -1);
    ints.set(10, -2);
    mvbo.region(sizeof(int) * 9, sizeof(int) * 2, "negatives");
    // now my favourite number!!!
    ints.set(11, 862);
    mvbo.region(sizeof(int) * 11, sizeof(int), "best number ever!");

    topaz_expect_assert(test_case, false, "Unexpected assertion whilst filling managed buffer regions...");
    // we should have filled the whole damn thing by now.
    topaz_expect(test_case, mvbo.regions_full(), "tz::gl::ManagedTerminalBuffer<Type>: ManagedTerminalBuffer failed to recognise that it should be full. Size: ", mvbo.size(), ", regions usage: ", mvbo.regions_usage());
    // we can defrag here and nothing should change.
    topaz_expect(test_case, !mvbo.defragment(), "tz::gl::ManagedTerminalBuffer<Type>::defragment() wrongly moved regions when it was already full.");
    
    // changed my mind! i fucking hate negative numbers. let's purge them all.
    mvbo.erase("negatives");
    // now regions should no longer fill the whole thing.
    topaz_expect(test_case, !mvbo.regions_full(), "tz::gl::ManagedTerminalBuffer<Type>: ManagedTerminalBuffer wrongly believes it still has full region usage after erasing a non-empty region.");
    // defragging should do something!
    topaz_expect(test_case, mvbo.defragment(), "tz::gl::ManagedTerminalBuffer<Type>::defragment() failed to move any regions when it should have. Size: ", mvbo.size(), ", region usage: ", mvbo.regions_usage());
    
    // ensure ordering remains correct!
    topaz_expect(test_case, mvbo[0] == "primes", "tz::gl::ManagedTerminalBuffer<Type>::defragment() Wrongly changed region ordering.");
    topaz_expect(test_case, mvbo[1] == "evens", "tz::gl::ManagedTerminalBuffer<Type>::defragment() Wrongly changed region ordering.");
    topaz_expect(test_case, mvbo[2] == "best number ever!", "tz::gl::ManagedTerminalBuffer<Type>::defragment() Wrongly changed region ordering.");
    
    // ensure elements remain consistent.
    auto region = mvbo["primes"];
    /*
    ints[0] = 3;
    ints[1] = 5;
    ints[2] = 7;
    ints[3] = 11;
    */
    for(std::size_t i = 0; i < 4; i++)
    {
        int cur = *region.block.get<int>(sizeof(int) * i);
        topaz_expect(test_case, ints[i] == cur, "Region elements after defragmentation didn't match. Expected ", ints[i], ", got ", cur);
    }
    topaz_expect_assert(test_case, false, "Unexpected assert");
    return test_case;
}

tz::test::Case nonterminal()
{
    tz::test::Case test_case("tz::gl::ManagedNonterminalBuffer Tests");
    tz::gl::Object o;

    std::size_t mvbo_id = o.emplace_managed_buffer<tz::gl::BufferType::Array>();
    tz::gl::ManagedVBO& mvbo = *o.get_managed<tz::gl::BufferType::Array>(mvbo_id);

    std::vector<int> data;
    // evens
    data.push_back(2);
    data.push_back(4);
    data.push_back(6);
    data.push_back(8);
    // odds
    data.push_back(9);
    data.push_back(11);
    data.push_back(13);
    data.push_back(15);

    // send it off!
    mvbo.resize(data.size() * sizeof(int));
    mvbo.send_range(data.begin(), data.end());
    // make regions
    mvbo.region(0, sizeof(int) * 4, "evens");
    mvbo.region(sizeof(int) * 4, sizeof(int) * 4, "odds");
    auto evens_region = mvbo["evens"];
    // ensure all makes sense.
    {
        tz::mem::Block blk = mvbo.map();
        // check first 4 ints to make sure they're the evens we expect.
        for(std::size_t i = 0; i < 4; i++)
        {
            int cur = *blk.get<int>(sizeof(int) * i);
            topaz_expect(test_case, cur == data[i], "ManagedNonterminalBuffer region access had unexpected value. Expected ", data[i], ", but got ", cur);
        }

        mvbo.unmap();
    }
    topaz_expect_assert(test_case, false, "Unexpected assert at some point during nonterminal managed buffer tests...");
    return test_case;
}

int main()
{
    tz::test::Unit managed;

    // We require topaz to be initialised.
    {
        tz::core::initialise("Managed Buffer Tests");
        managed.add(regions());
        managed.add(mock_mesh());
        managed.add(defragmentation());
        managed.add(nonterminal());
        tz::core::terminate();
    }
    return managed.result();
}