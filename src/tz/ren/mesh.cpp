#include "tz/ren/mesh.hpp"
#include "tz/gl/api/schedule.hpp"
#include "tz/gl/device.hpp"

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
		- vertex buffer
		- index buffer
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

	void mesh_renderer::append_to_render_graph()
	{
		auto hanval_ch = static_cast<tz::gl::eid_t>(static_cast<tz::hanval>(this->compute_pass.handle));
		auto hanval_rh = static_cast<tz::gl::eid_t>(static_cast<tz::hanval>(this->render_pass.handle));
		// compute pass happens first, then the render pass.
		tz::gl::get_device().render_graph().timeline.push_back(hanval_ch);
		tz::gl::get_device().render_graph().timeline.push_back(hanval_rh);
		// render pass depends on compute pass.
		tz::gl::get_device().render_graph().add_dependencies(this->render_pass, this->compute_pass);
	}

	mesh_renderer::compute_pass_t::compute_pass_t()
	{
		// we have a draw buffer which we write into upon render.
	}
}