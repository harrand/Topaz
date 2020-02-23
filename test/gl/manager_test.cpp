//
// Created by Harrand on 23/02/2020.
//

#include "test_framework.hpp"
#include "core/core.hpp"
#include "core/tz_glad/glad_context.hpp"
#include "gl/manager.hpp"

tz::gl::IndexedMesh square()
{
    tz::gl::IndexedMesh sq;
    // t1
    sq.vertices.push_back(tz::gl::Vertex{{{-0.5f, -0.5f, 0.0f}}, {{0.0f, 0.0f}}, {{}}, {{}}, {{}}});
    sq.vertices.push_back(tz::gl::Vertex{{{0.5f, -0.5f, 0.0f}}, {{1.0f, 0.0f}}, {{}}, {{}}, {{}}});
    sq.vertices.push_back(tz::gl::Vertex{{{0.5f, 0.5f, 0.0f}}, {{1.0f, 0.5f}}, {{}}, {{}}, {{}}});
    // t2
    sq.vertices.push_back(tz::gl::Vertex{{{-0.5f, -0.5f, 0.0f}}, {{0.0f, 0.0f}}, {{}}, {{}}, {{}}});
    sq.vertices.push_back(tz::gl::Vertex{{{0.5f, 0.5f, 0.0f}}, {{1.0f, 0.5f}}, {{}}, {{}}, {{}}});
    sq.vertices.push_back(tz::gl::Vertex{{{-0.5f, 0.5f, 0.0f}}, {{0.0f, 0.5f}}, {{}}, {{}}, {{}}});
    sq.indices = {0, 1, 2, 3, 4, 5};
    return sq;
}

tz::test::Case partition()
{
    tz::test::Case test_case("tz::gl::Manager Partition Tests");
    tz::gl::Manager m;

    // add the mesh, and then partition it into two triangles.
    tz::gl::Manager::Handle t1 = m.add_mesh(square());
    topaz_expect(test_case, m.get_number_of_vertices(t1) == 6, "Handle had unexpected number of vertices. Expected ", 6, ", got ", t1);
    tz::gl::Manager::Handle t2 = m.partition(t1, 3);
    // number of vertices must be conserved
    topaz_expect(test_case, m.get_number_of_vertices(t1) == 3, "Handle had unexpected number of vertices. Expected ", 3, ", got ", t1);
    topaz_expect(test_case, m.get_number_of_vertices(t2) == 3, "Handle had unexpected number of vertices. Expected ", 3, ", got ", t2);
    return test_case;
}

tz::test::Case split()
{
    tz::test::Case test_case("tz::gl::Manager Split Tests");
    tz::gl::Manager m;

    // add the mesh, and then split it into 3 sets of 2-vertex handles.
    tz::gl::Manager::Handle p1 = m.add_mesh(square());
    std::vector<tz::gl::Manager::Handle> handles = m.split(p1, 2);
    topaz_expect(test_case, handles.size() == 3, "Expected to split handle into ", 3, "daughters, but instead had ", handles.size(), " daughters");

    return test_case;
}

int main()
{
    tz::test::Unit manager;

    // We require topaz to be initialised.
    {
        tz::core::initialise("Manager Tests");
        manager.add(partition());
        tz::core::terminate();
    }
    return manager.result();
}