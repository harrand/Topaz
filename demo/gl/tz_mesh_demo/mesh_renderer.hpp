#ifndef TZ_DEMO_MESH_DEMO_MESH_RENDERER_HPP
#define TZ_DEMO_MESH_DEMO_MESH_RENDERER_HPP
#include "mesh.hpp"
#include "tz/core/matrix.hpp"
#include "tz/gl/device.hpp"
#include "tz/gl/renderer.hpp"
#include "shaders/texcount.tzsl"

using meshid_t = std::uint32_t;
using texid_t = std::uint32_t;

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

struct texture_data_t
{
	tz::vec3 tint = tz::vec3::filled(1.0f);
	texid_t texid = 0;
};

constexpr std::size_t object_attached_texture_count = TEX_COUNT;

struct transform_t
{
	tz::vec3 pos = tz::vec3::zero();
	tz::vec3 rot = tz::vec3::zero();
	tz::vec3 scale = tz::vec3::filled(1.0f);
};

struct drawdata_element_t
{
	tz::mat4 model;
	std::array<texture_data_t, object_attached_texture_count> textures;
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
// meshes live in ib and vb. static resources, written/resized on-demand. you can add new meshes whenever you want.
// draw list represents an element using one of the meshes. also has a transform.
// every frame, compute shader is invoked which takes the draw list data and populates the draw commands via draw_indirect_count.
// the the graphics renderer is sent with the draw commands and we're good to go.
class mesh_renderer
{
public:
	mesh_renderer(std::size_t max_texture_count);

	meshid_t add_mesh(mesh_t mesh, const char* name = "Untitled Mesh");
	// rgba32 only.
	texid_t add_texture(unsigned int width, unsigned int height, std::span<const std::byte> imgdata);

	void push_back_timeline() const;
	void add_to_draw_list(meshid_t mesh, transform_t transform = {}, texid_t tex = 0);
	void dbgui();
	void update(float dt);
private:
	void append_mesh_to_buffers(const mesh_t& mesh);
	void append_meshid_to_draw_buffer(meshid_t mesh, transform_t transform, texid_t tex);
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
	std::vector<tz::gl::resource_handle> textures = {};
	tz::gl::renderer_handle ch = tz::nullhand;
	tz::gl::renderer_handle rh = tz::nullhand;
	std::vector<mesh_renderer_entry> entries = {};
	std::vector<const char*> entry_names = {};
	std::vector<meshid_t> draw_list = {};
	std::uint32_t cumulative_vertex_count = 0;
	std::size_t texture_cursor = 0;
	bool allow_controls = true;
	float camera_speed = 10.0f;
	float camera_rot_speed = 1.0f;
	struct camera_intermediate_data_t
	{
		tz::vec3 pos = {0.0f, 0.5f, 0.0f};
		tz::vec3 rot = {0.0f, 0.0f, 0.0f};
	} camera_intermediate_data;
};


#endif // TZ_DEMO_MESH_DEMO_MESH_RENDERER_HPP
