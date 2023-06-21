#include "mesh_renderer.hpp"
#include "tz/gl/imported_shaders.hpp"
#include "tz/gl/resource.hpp"
#include "tz/dbgui/dbgui.hpp"

#include ImportedShaderHeader(mesh3d, vertex)
#include ImportedShaderHeader(mesh3d, fragment)

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
	ImGui::Text("well met");
}
