#include "tz/ren/mesh.hpp"
#include "tz/gl/api/schedule.hpp"
#include "tz/gl/device.hpp"
#include "tz/gl/draw.hpp"
#include "tz/gl/resource.hpp"
#include "tz/core/matrix.hpp"
#include "tz/dbgui/dbgui.hpp"

namespace tz::ren
{
	/* mesh_renderer implementation details
	   ====================================

	   - there are two large buffers, one containing vertices, and the other indices.
	   - when a mesh is added, its vertices and indices are dumped into the massive array, the locations of the vertices and indices returned as a mesh_locator.
	   - when a mesh is removed, its vertices and indices are cleared to zero, and free to be filled by another mesh. however, this is prone to fragmentation.
	   - when a new drawable is added it selects which mesh to use, and adds its locator to the draw list.
	   	- the draw list is simply a big list of all the mesh locators that need to be drawn, and in what order. it also knows its own size

	   compute pass
	   ============
	   resources:
	   - draw list (containing the mesh locators from before)
	   - draw indirect buffer (for use by renderer)
	   when: happens once every frame
	   1. iterate through the entire draw list.
	   2. for each mesh locator in the draw list,
	   generate an equivalent indexed indirect draw
	   command, and write it into the corresponding
	   location within the draw indirect buffer.
	   3. set the draw indirect buffer's draw-count to the number of mesh locators within the draw list.

		render pass
		===========
		resources:
		- index buffer
		- vertex buffer
		- object buffer (contains the model matrix and list of texture ids used by each drawable)
		- draw indirect buffer (owned by the compute pass)
		- camera buffer, containing camera data.
		- an array of textures.
		when: happens once every frame, after the compute pass completes.
		1. retrieve the current vertex from the vertex buffer. remember
		that in::vertex_id is actually the index_id in TZSL.
		2. retrieve the texture and model matrix info from the object
		buffer using the draw_id.
		3. combine the model matrix with the projection and view matrices
		from the camera buffer.
		4. pass the relevant texture ids to the fragment shader.
		--- vertex shading occurs... ---
		--- fragment shading occurs... ----

	*/

	// describes the location and dimensions of a mesh within the giant vertex and index buffers.
	struct mesh_locator
	{
		// how far into the vertex buffer does this mesh's vertices live?
		std::uint32_t vertex_offset = 0;
		// how many vertices does this mesh have?
		std::uint32_t vertex_count = 0;
		// how far into the index buffer does this mesh's indices live?
		std::uint32_t index_offset = 0;	
		// how many indices does this mesh have?
		std::uint32_t index_count = 0;
		// X, where all indices of this mesh are between 0 and X.
		std::uint32_t max_index_value = 0;
	};
	// only support 64 meshes being drawn at a time. todo: dynamic resize?
	constexpr std::size_t max_drawn_meshes = 64;

	// a draw list contains all the meshes that need to be drawn in the right order.
	// the compute pass will take this list and populate the draw-indirect buffer,
	// which the renderer will the use to draw everything.
	struct draw_list
	{
		std::uint32_t draw_count = 0;
		std::array<mesh_locator, max_drawn_meshes> meshes;
	};

	// represents one of the textures bound to an object (drawable)
	struct texture_locator
	{
		// colour multiplier on the sampled texel
		tz::vec3 colour_tint = tz::vec3::filled(1.0f);
		// id within the overarching texture resource array to be sampled.
		std::uint32_t texture_id = static_cast<std::uint32_t>(-1);
	};

	// represents the data of an object (drawable).
	struct object_data
	{
		// represents the transform of the drawable, in world space.
		tz::mat4 model;
		// array of bound textures. they all do not have to be used. no indication on whether they are colour, normal map, etc...
		std::array<texture_locator, mesh_renderer_max_tex_count> bound_textures = {};
	};

	// represents the data of the camera.
	struct camera_data
	{
		tz::mat4 view = tz::mat4::identity();
		tz::mat4 projection = tz::mat4::identity();
	};

	mesh_renderer::mesh_renderer(unsigned int total_textures):
	compute_pass(),
	render_pass(this->compute_pass.handle, this->compute_pass.draw_indirect_buffer, total_textures)
	{

	}

	void mesh_renderer::append_to_render_graph()
	{
		auto hanval_ch = static_cast<tz::gl::eid_t>(static_cast<tz::hanval>(this->compute_pass.handle));
		auto hanval_rh = static_cast<tz::gl::eid_t>(static_cast<tz::hanval>(this->render_pass.handle));
		// compute pass happens first, then the render pass.
		tz::gl::get_device().render_graph().timeline.push_back(hanval_ch);
		tz::gl::get_device().render_graph().timeline.push_back(hanval_rh);
		// render pass depends on compute pass.
		tz::gl::get_device().render_graph().add_dependencies(this->render_pass.handle, this->compute_pass.handle);
	}

	void mesh_renderer::dbgui()
	{
		this->dbgui_impl();
	}

	mesh_renderer::compute_pass_t::compute_pass_t()
	{
		tz::gl::renderer_info cinfo;
		// todo: shaders
		struct draw_indirect_buffer_data
		{
			std::uint32_t count;
			std::array<tz::gl::draw_indexed_indirect_command, max_drawn_meshes> cmds;
		};
		// create draw indirect buffer. one draw cmd per mesh locator, upto our max.
		// todo: make this resizeable?
		this->draw_indirect_buffer = cinfo.add_resource(tz::gl::buffer_resource::from_one
		(
			draw_indirect_buffer_data{},
			{
				.flags = {tz::gl::resource_flag::draw_indirect_buffer}
			}
		));
		// same with the draw list
		// note: dynamic access to avoid a renderer edit on a change in drawable.
		// todo: is the above comment on dynamic access worthwhile? this way we
		// dont take a renderer edit hit when a new drawable is added iirc
		this->draw_list_buffer = cinfo.add_resource(tz::gl::buffer_resource::from_one
		(
			draw_list{},
			{
				.access = tz::gl::resource_access::dynamic_access
			}
		));
		cinfo.debug_name("Mesh Renderer - Compute Pass");
		this->handle = tz::gl::get_device().create_renderer(cinfo);
	}

	void mesh_renderer::compute_pass_t::dbgui()
	{
		
	}

	mesh_renderer::render_pass_t::render_pass_t(tz::gl::renderer_handle compute_pass, tz::gl::resource_handle compute_draw_indirect_buffer, unsigned int total_textures)
	{
		// we have a draw buffer which we write into upon render.
		tz::gl::renderer_info rinfo;
		// todo: shaders
		rinfo.set_options({tz::gl::renderer_option::draw_indirect_count});
		// index buffer (initially contains a single index. empty buffers should be a thing aaaa)
		this->index_buffer = rinfo.add_resource(tz::gl::buffer_resource::from_one
		(
			std::uint32_t{0},
			{
				.flags = {tz::gl::resource_flag::index_buffer}
			}
		));
		// vertex buffer (initially contains a single vertex. aaaaa)
		this->vertex_buffer = rinfo.add_resource(tz::gl::buffer_resource::from_one
		(
			std::byte{0}
		));
		// object buffer (contains textures, position data etc)
		std::array<object_data, max_drawn_meshes> initial_object_data = {};
		this->object_buffer = rinfo.add_resource(tz::gl::buffer_resource::from_many
		(
			initial_object_data,
			{
				.access = tz::gl::resource_access::dynamic_access
			}
		));
		// camera buffer
		this->camera_buffer = rinfo.add_resource(tz::gl::buffer_resource::from_one
		(
			camera_data{},
			{
				.access = tz::gl::resource_access::dynamic_access
			}
		));
		// draw indirect buffer (resource reference -> compute pass)
		this->draw_indirect_buffer_ref = rinfo.ref_resource(compute_pass, compute_draw_indirect_buffer);
		// textures
		this->textures.resize(total_textures);
		for(std::size_t i = 0; i < total_textures; i++)
		{
			this->textures[i] = rinfo.add_resource(tz::gl::image_resource::from_uninitialised
			({
				.format = tz::gl::image_format::RGBA32,
				.dimensions = {1u, 1u},
				.flags = {tz::gl::resource_flag::image_wrap_repeat}
			}));
		}

		// finally, hook up the relevant resources as the special buffers.
		rinfo.state().graphics.draw_buffer = this->draw_indirect_buffer_ref;
		rinfo.state().graphics.index_buffer = this->index_buffer;
		// create renderer.
		rinfo.debug_name("Mesh Renderer - Render Pass");
		this->handle = tz::gl::get_device().create_renderer(rinfo);
	}

	void mesh_renderer::render_pass_t::dbgui()
	{
		tz::gl::renderer& ren = tz::gl::get_device().get_renderer(this->handle);

		ImGui::Separator();
		ImGui::TextColored(ImVec4{1.0f, 0.3f, 0.3f, 1.0f}, "VERTEX DATA");
		std::size_t vertex_count = ren.get_resource(this->vertex_buffer)->data_as<const mesh_renderer::vertex_t>().size();
		std::size_t index_count = ren.get_resource(this->index_buffer)->data_as<const index>().size();
		ImGui::Text("%zu vertices, %zu indices (%zu triangles)", vertex_count, index_count, (index_count / 3));

		ImGui::Separator();
		ImGui::TextColored(ImVec4{1.0f, 0.3f, 0.3f, 1.0f}, "OBJECT DATA");
		std::size_t object_count = ren.get_resource(this->object_buffer)->data_as<const object_data>().size();
		static int object_id = 0;
		if(object_count > 0)
		{
			ImGui::VSliderInt("Object", ImVec2{18.0f, 160.0f}, &object_id, 0, object_count);
			object_data& obj = ren.get_resource(this->object_buffer)->data_as<object_data>()[object_id];
			ImGui::Text("Object %d", object_id);	
			// TODO: object information display
			(void)obj;
		}

		ImGui::Separator();
		ImGui::TextColored(ImVec4{1.0f, 0.3f, 0.3f, 1.0f}, "CAMERA DATA");
		auto& cam = ren.get_resource(this->camera_buffer)->data_as<camera_data>().front();
		ImGui::Text("camera data placeholder...");
		ImGui::Separator();
	}

	// dbgui

	void mesh_renderer::dbgui_impl()
	{
		// before trying to re-understand this, read the explanation of
		// how the renderer works at the top of this file. seriously.

		// possible implementation (feel free to do something else)
		// probably worth making a few tab bars.
		// unused | impl details
		// impl details ->
		// compute pass | render pass
		// and then for each pass, expose dynamic resources, typical stuff etc...
		if(ImGui::BeginTabBar("Render or Compute"))
		{
			if(ImGui::BeginTabItem("Compute Pass"))
			{
				this->compute_pass.dbgui();
			}
			if(ImGui::BeginTabItem("Render Pass"))
			{
				this->render_pass.dbgui();
			}
		}
	}
}