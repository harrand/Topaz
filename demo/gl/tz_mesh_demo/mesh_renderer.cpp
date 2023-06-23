#include "mesh_renderer.hpp"
#include "tz/gl/imported_shaders.hpp"
#include "tz/gl/resource.hpp"
#include "tz/gl/draw.hpp"
#include "tz/dbgui/dbgui.hpp"

#include ImportedShaderHeader(mesh3d, vertex)
#include ImportedShaderHeader(mesh3d, fragment)
#include ImportedShaderHeader(preprocess, compute)

meshid_t mesh_renderer_entry::meshid_internal_count = 0;

mesh_renderer::mesh_renderer():
ch([this]()
{
	tz::gl::renderer_info cinfo;
	cinfo.shader().set_shader(tz::gl::shader_stage::compute, ImportedShaderSource(preprocess, compute));
	struct draw_indirect_count_data
	{
		std::uint32_t count;
		std::array<tz::gl::draw_indexed_indirect_command, max_draw_count> cmd;
	};
	this->db = cinfo.add_resource(tz::gl::buffer_resource::from_one(draw_indirect_count_data{},
	{
		.flags = {tz::gl::resource_flag::draw_indirect_buffer}
	}));
	this->meshref_buf = cinfo.add_resource(tz::gl::buffer_resource::from_one(meshref_storage_t{},
	{
		.access = tz::gl::resource_access::dynamic_access
	}));
	return tz::gl::get_device().create_renderer(cinfo);
}()),
rh([this]()
{
	tz::gl::renderer_info rinfo;
	rinfo.shader().set_shader(tz::gl::shader_stage::vertex, ImportedShaderSource(mesh3d, vertex));
	rinfo.shader().set_shader(tz::gl::shader_stage::fragment, ImportedShaderSource(mesh3d, fragment));
	rinfo.set_options({tz::gl::renderer_option::draw_indirect_count});
	this->ib = rinfo.add_resource(tz::gl::buffer_resource::from_one(std::uint32_t{0},
	{
		.flags = {tz::gl::resource_flag::index_buffer}
	}));
	this->vb = rinfo.add_resource(tz::gl::buffer_resource::from_one(std::byte{0}));
	this->dbref = rinfo.ref_resource(this->ch, this->db);
	rinfo.state().graphics.draw_buffer = this->dbref;
	rinfo.state().graphics.index_buffer = this->ib;
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

void mesh_renderer::push_back_timeline() const
{
	auto hanval_ch = static_cast<tz::gl::eid_t>(static_cast<tz::hanval>(this->ch));
	auto hanval_rh = static_cast<tz::gl::eid_t>(static_cast<tz::hanval>(this->rh));
	tz::gl::get_device().render_graph().timeline.push_back(hanval_ch);
	tz::gl::get_device().render_graph().timeline.push_back(hanval_rh);
	tz::gl::get_device().render_graph().add_dependencies(this->rh, this->ch);
}

void mesh_renderer::add_to_draw_list(meshid_t mesh)
{
	this->append_meshid_to_draw_buffer(mesh);
	this->draw_list.push_back(mesh);
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
	std::size_t vtx_bytes = 0, idx_bytes = 0;
	// if entries size <= 1, then we're using the default placeholder values for ib and vb (we cant have zero sized buffers lmao), so we rewrite the whole thing and resize to just include the new data.
	if(this->entries.size() > 1)
	{
		vtx_bytes += ren.get_resource(this->vb)->data().size_bytes();
		idx_bytes += ren.get_resource(this->ib)->data().size_bytes();
	}
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

void mesh_renderer::append_meshid_to_draw_buffer(meshid_t mesh)
{
	// figure out what the draw command is gonna be.
	mesh_reference ref = this->get_reference(mesh);
	meshref_storage_t& storage = tz::gl::get_device().get_renderer(this->ch).get_resource(this->meshref_buf)->data_as<meshref_storage_t>().front();
	storage.draw_count = this->draw_list.size() + 1;
	storage.meshref_storage[this->draw_list.size()] = ref;
}

mesh_reference mesh_renderer::get_reference(meshid_t mesh) const
{
	unsigned int voffset = 0;
	unsigned int ioffset = 0;
	for(const mesh_renderer_entry& entry : this->entries)
	{
		if(entry.meshid == mesh)
		{
			return
			{
				.vtx_offset = voffset,
				.idx_offset = ioffset,
				.entry = entry
			};
		}
	}
	tz::error("Could not find mesh reference. Did you delete it?");
	return {};
}
