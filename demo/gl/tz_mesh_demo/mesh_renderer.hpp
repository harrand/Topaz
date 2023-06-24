#ifndef TZ_DEMO_MESH_DEMO_MESH_RENDERER_HPP
#define TZ_DEMO_MESH_DEMO_MESH_RENDERER_HPP
#include "mesh.hpp"
#include "tz/core/matrix.hpp"
#include "tz/gl/device.hpp"
#include "tz/gl/renderer.hpp"

using meshid_t = std::uint32_t;

struct mesh_renderer_entry
{
	static meshid_t meshid_internal_count;
	meshid_t meshid = meshid_internal_count++;
	std::uint32_t vtx_count;
	std::uint32_t idx_count;
	std::uint32_t max_idx;
};

struct mesh_reference
{
	std::uint32_t vtx_offset = 0;
	std::uint32_t idx_offset = 0;
	mesh_renderer_entry entry = {};
};

constexpr std::size_t max_draw_count = 64;

struct meshref_storage_t
{
	std::uint32_t draw_count = 0;
	std::array<mesh_reference, max_draw_count> meshref_storage;
};

struct drawdata_element_t
{
	tz::mat4 model;
};

struct drawdata_storage_t
{
	std::array<drawdata_element_t, max_draw_count> draws;
};

struct camera_data_t
{
	tz::mat4 v = tz::mat4::identity();
	tz::mat4 p = tz::mat4::identity();
};

// responsible for rendering meshes.
class mesh_renderer
{
public:
	mesh_renderer();

	meshid_t add_mesh(mesh_t mesh, const char* name = "Untitled Mesh");

	void push_back_timeline() const;
	void add_to_draw_list(meshid_t mesh);
	void dbgui();
private:
	void append_mesh_to_buffers(const mesh_t& mesh);
	void append_meshid_to_draw_buffer(meshid_t mesh);
	mesh_reference get_reference(meshid_t mesh) const;
	void write_camera_buffer();
	// draw indirect buffer
	tz::gl::resource_handle db = tz::nullhand;
	tz::gl::resource_handle meshref_buf = tz::nullhand;
	tz::gl::resource_handle drawdata_buf = tz::nullhand;
	tz::gl::resource_handle camera_buf = tz::nullhand;
	// vertex buffer
	tz::gl::resource_handle vb = tz::nullhand;
	// index buffer
	tz::gl::resource_handle ib = tz::nullhand;
	tz::gl::resource_handle dbref = tz::nullhand;
	tz::gl::renderer_handle ch = tz::nullhand;
	tz::gl::renderer_handle rh = tz::nullhand;
	std::vector<mesh_renderer_entry> entries = {};
	std::vector<const char*> entry_names = {};
	std::vector<meshid_t> draw_list = {};
	struct camera_intermediate_data_t
	{
		tz::vec3 pos = {0.2f, 0.2f, 2.5f};
		tz::vec3 rot = {0.0f, 0.0f, 0.0f};
	} camera_intermediate_data;
};


#endif // TZ_DEMO_MESH_DEMO_MESH_RENDERER_HPP
