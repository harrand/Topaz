#include "mesh_renderer.hpp"
#include "tz/gl/imported_shaders.hpp"
#include "tz/gl/resource.hpp"
#include "tz/dbgui/dbgui.hpp"

#include ImportedShaderHeader(mesh3d, vertex)
#include ImportedShaderHeader(mesh3d, fragment)

meshid_t mesh_renderer_entry::meshid_internal_count = 0;

mesh_renderer::mesh_renderer():
rh([this]()
{
	tz::gl::renderer_info rinfo;
	rinfo.shader().set_shader(tz::gl::shader_stage::vertex, ImportedShaderSource(mesh3d, vertex));
	rinfo.shader().set_shader(tz::gl::shader_stage::fragment, ImportedShaderSource(mesh3d, fragment));
	this->vb = rinfo.add_resource(tz::gl::buffer_resource::from_one(std::byte{0}));
	this->ib = rinfo.add_resource(tz::gl::buffer_resource::from_one(std::uint32_t{0},
	{
		.flags = {tz::gl::resource_flag::index_buffer}
	}));
	this->mb = rinfo.add_resource(tz::gl::buffer_resource::from_one(std::byte{0}));
	rinfo.state().graphics.tri_count = 1;
	rinfo.debug_name("Mesh Renderer");
	return tz::gl::get_device().create_renderer(rinfo);
}())
{

}

meshid_t mesh_renderer::add_mesh(mesh_t mesh, const char* name)
{
	this->entries.push_back
	({
		.vtx_count = static_cast<unsigned int>(mesh.vertices.size()),
	  	.idx_count = static_cast<unsigned int>(mesh.indices.size()),
	  	.mesh_name = name
	});
	this->append_mesh_to_buffers(mesh);
	return this->entries.back().meshid;
}

tz::gl::renderer_handle mesh_renderer::get() const
{
	return this->rh;
}

void mesh_renderer::push_back_timeline() const
{
	auto hanval = static_cast<tz::gl::eid_t>(static_cast<tz::hanval>(this->get()));
	tz::gl::get_device().render_graph().timeline.push_back(hanval);
}

void mesh_renderer::dbgui()
{
	ImGui::Text("Mesh Count: %zu", this->entries.size());
	if(this->entries.size())
	{
		static int mesh_view_id = 0;
		ImGui::SliderInt("Mesh Viewer", &mesh_view_id, 0, this->entries.size() - 1);
		mesh_view_id = std::min(static_cast<std::size_t>(mesh_view_id), this->entries.size());

		const mesh_renderer_entry& entry = this->entries[mesh_view_id];
		ImGui::Indent();
		ImGui::TextColored(ImVec4{1.0f, 0.6f, 0.6f, 1.0f}, "%s (%zu)", entry.mesh_name, entry.meshid);
		ImGui::Text("vertex count: %zu (%zuB)", entry.vtx_count, (entry.vtx_count * sizeof(vertex_t)));
		ImGui::Text("index count: %zu (%zuB)", entry.idx_count, (entry.idx_count * sizeof(std::uint32_t)));
		ImGui::Unindent();
	}
}

void mesh_renderer::append_mesh_to_buffers(const mesh_t& mesh)
{
	// find out how much vertex and index bytes we need.
	tz::gl::renderer& ren = tz::gl::get_device().get_renderer(this->rh);
	std::size_t vtx_bytes = ren.get_resource(this->vb)->data().size_bytes();
	std::size_t idx_bytes = ren.get_resource(this->ib)->data().size_bytes();
	std::size_t old_vtx_size = vtx_bytes;
	std::size_t old_idx_size = idx_bytes;
	// add it to whatever we have now.
	vtx_bytes += mesh.vertices.size() * sizeof(vertex_t);
	idx_bytes += mesh.indices.size() * sizeof(std::uint32_t);
	// do the necessary resizes.
	ren.edit(tz::gl::RendererEditBuilder{}
		.buffer_resize({.buffer_handle = this->vb, .size = vtx_bytes})
		.buffer_resize({.buffer_handle = this->ib, .size = idx_bytes})
		.build()
	);
	// then the writes.
	std::span<const vertex_t> new_vtx_data = mesh.vertices;
	std::span<const std::uint32_t> new_idx_data = mesh.indices;
	ren.edit(tz::gl::RendererEditBuilder{}
		.write
		({
			.resource = this->vb,
	   		.data = std::as_bytes(new_vtx_data),
	   		.offset = old_vtx_size
		})
   		.write
		({
			.resource = this->ib,
	   		.data = std::as_bytes(new_idx_data),
	   		.offset = old_idx_size
		})
   		.build()
	);
}
