#ifndef TZ_DEMO_MESH_DEMO_MESH_RENDERER_HPP
#define TZ_DEMO_MESH_DEMO_MESH_RENDERER_HPP
#include "mesh.hpp"
#include "tz/gl/device.hpp"
#include "tz/gl/renderer.hpp"

using meshid_t = std::size_t;

struct mesh_renderer_entry
{
	static meshid_t meshid_internal_count;
	meshid_t meshid = meshid_internal_count++;
	unsigned int vtx_count;
	unsigned int idx_count;
	const char* mesh_name;
};

// responsible for rendering meshes.
class mesh_renderer
{
public:
	mesh_renderer();

	meshid_t add_mesh(mesh_t mesh, const char* name = "Untitled Mesh");

	tz::gl::renderer_handle get() const;
	void push_back_timeline() const;
	void dbgui();
private:
	void append_mesh_to_buffers(const mesh_t& mesh);
	// vertex buffer
	tz::gl::resource_handle vb = tz::nullhand;
	// index buffer
	tz::gl::resource_handle ib = tz::nullhand;
	// mesh meta buffer
	tz::gl::resource_handle mb = tz::nullhand;
	tz::gl::renderer_handle rh = tz::nullhand;
	std::vector<mesh_renderer_entry> entries = {};
};


#endif // TZ_DEMO_MESH_DEMO_MESH_RENDERER_HPP
