#include "core/tz.hpp"
#include "core/assert.hpp"
#include "gl/input.hpp"

int main()
{
	tz::initialise({"tz_mesh_test", tz::Version{1, 0, 0}, tz::info()}, tz::ApplicationType::Headless);
	{
		tz::gl::Mesh mesh;
		mesh.vertices =
		{
			tz::gl::Vertex{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}, {}, {}, {}},
			tz::gl::Vertex{{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}, {}, {}, {}},
			tz::gl::Vertex{{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f}, {}, {}, {}}
		};
		mesh.indices =
		{
			0, 1, 2
		};
		tz::gl::MeshInput mesh_input{mesh};

		{
			const std::size_t expected_vertices_size = mesh.vertices.length() * sizeof(tz::gl::Vertex);
			const std::size_t a = mesh_input.get_vertex_bytes().size();
			const std::size_t b = mesh_input.get_vertex_bytes().size_bytes();
			tz_assert(a == b && b == expected_vertices_size, "Unexpected MeshInput Vertices Dimensions");
		}
		{
			const std::size_t expected_indices_size = mesh.indices.length();
			const std::size_t expected_indices_bytes = mesh.indices.length() * sizeof(unsigned int);
			tz_assert(mesh_input.get_indices().size() == expected_indices_size && mesh_input.get_indices().size_bytes() == expected_indices_bytes, "Unexpected MeshInput Indices Dimensions");
		}
	}
	tz::terminate();
}