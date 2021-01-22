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

TZ_TEST_BEGIN(partition)
	tz::gl::Manager m;

	// add the mesh, and then partition it into two triangles.
	tz::gl::Manager::Handle t1 = m.add_mesh(square());
	std::size_t offset = m.get_vertices_offset(t1);
	topaz_expect(m.get_number_of_vertices(t1) == 6, "Handle had unexpected number of vertices. Expected ", 6, ", got ", t1);
	tz::gl::Manager::Handle t2 = m.partition(t1, 3);
	// number of vertices must be conserved
	topaz_expect(m.get_vertices_offset(t1) == offset, "Handle had unexpected offset. Expected ", offset, ", got ", m.get_vertices_offset(t1));
	topaz_expect(m.get_vertices_offset(t2) == offset + 3, "Handle had unexpected offset. Expected ", offset + 3, ", got ", m.get_vertices_offset(t2));
	topaz_expect(m.get_number_of_vertices(t1) == 3, "Handle had unexpected number of vertices. Expected ", 3, ", got ", m.get_number_of_vertices(t1));
	topaz_expect(m.get_number_of_vertices(t2) == 3, "Handle had unexpected number of vertices. Expected ", 3, ", got ", m.get_number_of_vertices(t2));
TZ_TEST_END

TZ_TEST_BEGIN(split)
	tz::gl::Manager m;

	// add the mesh, and then split it into 3 sets of 2-vertex handles.
	tz::gl::Manager::Handle p1 = m.add_mesh(square());
	constexpr std::size_t split_amt = 2;
	std::vector<tz::gl::Manager::Handle> handles = m.split(p1, split_amt);
	topaz_expect(handles.size() == 3, "Expected to split handle into ", 3, "daughters, but instead had ", handles.size(), " daughters");
	for(std::size_t i = 0; i < handles.size(); i++)
	{
		tz::gl::Manager::Handle px = handles[i];
		// All should have same number of vertices.
		topaz_expect(m.get_number_of_vertices(px) == 2, "Handle ", px, " expected to contain ", 2, " vertices, but it apparantly contains ", m.get_number_of_vertices(px));
		// Should have stepping offsets
		std::size_t expected_offset = m.get_vertices_offset(p1) + (2 * i);
		topaz_expect(m.get_vertices_offset(px) == expected_offset, "Handle ", px, " expected to have vertex offset of ", expected_offset, ", but it has an offset of ", m.get_vertices_offset(px));
	}
TZ_TEST_END

int main()
{
	tz::test::Unit manager;

	// We require topaz to be initialised.
	{
		tz::core::initialise("Manager Tests", tz::core::invisible_tag);
		manager.add(partition());
		manager.add(split());
		tz::core::terminate();
	}
	return manager.result();
}