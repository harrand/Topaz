#include "tz/ren/mesh.hpp"
#include "tz/gl/api/schedule.hpp"
#include "tz/gl/device.hpp"
#include "tz/gl/draw.hpp"
#include "tz/gl/resource.hpp"
#include "tz/gl/imported_shaders.hpp"
#include "tz/core/matrix.hpp"
#include "tz/dbgui/dbgui.hpp"
#include "tz/io/gltf.hpp"
#include "tz/core/matrix_transform.hpp"
#include "tz/core/job/job.hpp"
#include "tz/core/profile.hpp"
#include <filesystem>

#include ImportedShaderHeader(mesh, compute)
#include ImportedShaderHeader(mesh, vertex)
#include ImportedShaderHeader(mesh, fragment)

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

	// only support a certain number of meshes being drawn at a time. todo: dynamic resize?
	constexpr std::size_t max_drawn_meshes = 256;

	// a draw list contains all the meshes that need to be drawn in the right order.
	// the compute pass will take this list and populate the draw-indirect buffer,
	// which the renderer will the use to draw everything.
	struct draw_list
	{
		std::uint32_t draw_count = 0;
		std::array<mesh_locator, max_drawn_meshes> meshes;
	};

	// represents the data of the camera.
	struct camera_data
	{
		tz::mat4 view = tz::mat4::identity();
		tz::mat4 projection = tz::perspective(3.14159f * 0.5f, 1920.0f/1080.0f, 0.1f, 1000.0f);
	};

	mesh_renderer::mesh_renderer(unsigned int total_textures):
	compute_pass(),
	render_pass(this->compute_pass.handle, this->compute_pass.draw_indirect_buffer, total_textures)
	{
		this->add_mesh({});
	}

	mesh_renderer::mesh_handle empty_mesh()
	{
		return static_cast<tz::hanval>(0);
	}

	mesh_renderer::mesh_handle mesh_renderer::add_mesh(mesh_renderer::mesh_t m)
	{
		std::size_t hanval = this->render_pass.meshes.size();
		mesh_locator locator = this->add_mesh_impl(m);
		this->render_pass.meshes.push_back(locator);
		return static_cast<tz::hanval>(hanval);
	}

	mesh_renderer::object_handle mesh_renderer::add_object(mesh_handle m, object_data data)
	{
		std::size_t hanval = this->draw_count();
		auto mesh_id = static_cast<std::size_t>(static_cast<tz::hanval>(m));
		// draw list at this position is now equal to the associated mesh_locator.
		tz::assert(hanval < this->compute_pass.get_draw_list_meshes().size(), "ran out of objects! can only have %zu", this->compute_pass.get_draw_list_meshes().size());
		this->compute_pass.get_draw_list_meshes()[hanval] = this->render_pass.meshes[mesh_id];
		// now need to fill the object data
		this->render_pass.get_object_datas()[hanval] = data;
		// finally, increment the draw count.
		this->compute_pass.set_draw_count(this->compute_pass.get_draw_count() + 1);

		return static_cast<tz::hanval>(hanval);
	}

	mesh_renderer::texture_handle mesh_renderer::add_texture(tz::vec2ui dimensions, std::span<const std::byte> image_data)
	{
		#if TZ_DEBUG
			std::size_t sz = tz::gl::pixel_size_bytes(tz::gl::image_format::RGBA32) * dimensions[0] * dimensions[1];
			tz::assert(image_data.size_bytes() == sz, "Unexpected image data length. Expected %zuB, but was %zuB", sz, image_data.size_bytes());
		#endif
		tz::assert(this->render_pass.texture_cursor < this->render_pass.textures.size(), "ran out of textures");
		tz::gl::resource_handle imgh = this->render_pass.textures[this->render_pass.texture_cursor];

		tz::gl::get_device().get_renderer(this->render_pass.handle).edit
		(
			tz::gl::RendererEditBuilder{}
			.image_resize({.image_handle = imgh, .dimensions = dimensions})
			.write({.resource = imgh, .data = image_data})
			.build()
		);

		return static_cast<tz::hanval>(this->render_pass.texture_cursor++);
	}

	std::size_t mesh_renderer::mesh_count() const
	{
		return this->render_pass.meshes.size();
	}

	std::size_t mesh_renderer::draw_count() const
	{
		return this->compute_pass.get_draw_count();
	}

	void mesh_renderer::clear()
	{
		// does not change capacities of any buffers.
		this->clear_draws();
		// next mesh to be added will simply be right at the front and begin overwriting.
		this->render_pass.meshes.clear();
		this->render_pass.cumulative_vertex_count = 0;
		this->render_pass.texture_cursor = 0;
	}

	void mesh_renderer::clear_draws()
	{
		this->compute_pass.set_draw_count(0);
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
		cinfo.shader().set_shader(tz::gl::shader_stage::compute, ImportedShaderSource(mesh, compute));
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

	std::span<const mesh_locator> mesh_renderer::compute_pass_t::get_draw_list_meshes() const
	{
		return tz::gl::get_device().get_renderer(this->handle).get_resource(this->draw_list_buffer)->data_as<const draw_list>().front().meshes;
	}

	std::span<mesh_locator> mesh_renderer::compute_pass_t::get_draw_list_meshes()
	{
		return tz::gl::get_device().get_renderer(this->handle).get_resource(this->draw_list_buffer)->data_as<draw_list>().front().meshes;
	}

	std::uint32_t mesh_renderer::compute_pass_t::get_draw_count() const
	{
		return tz::gl::get_device().get_renderer(this->handle).get_resource(this->draw_list_buffer)->data_as<draw_list>().front().draw_count;
	}

	void mesh_renderer::compute_pass_t::set_draw_count(std::uint32_t new_draw_count)
	{
		tz::gl::get_device().get_renderer(this->handle).get_resource(this->draw_list_buffer)->data_as<draw_list>().front().draw_count = new_draw_count;
	}

	void imgui_helper_tooltip(const char* msg)
	{
		ImGui::SameLine();
		ImGui::TextDisabled("(?)");
		if(ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(msg);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	void mesh_renderer::compute_pass_t::dbgui()
	{
		ImGui::Separator();
		ImGui::TextColored(ImVec4{1.0f, 0.3f, 0.3f, 1.0f}, "DRAW LIST");
		auto draw_list = this->get_draw_list_meshes();
		static int draw_id = 0;
		static bool only_show_active_draws = true;
		ImGui::Checkbox("Only display active draws", &only_show_active_draws);
		int draw_list_limit = draw_list.size();
		if(only_show_active_draws)
		{
			draw_list_limit = this->get_draw_count();
		}
		if(draw_list_limit > 0)
		{
			constexpr float slider_height = 160.0f;
			ImGui::VSliderInt("##drawelem", ImVec2{18.0f, slider_height}, &draw_id, 0, draw_list_limit - 1);
			std::string drawname = "Draw " + std::to_string(draw_id);
			bool is_active_draw = std::cmp_less(draw_id, this->get_draw_count());
			if(!is_active_draw)
			{
				drawname = "Inactive " + drawname;
			}
			ImGui::SameLine();

			// add slight horizontal spacing between slider and child.
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
			const mesh_locator& mloc = draw_list[draw_id];
			if(ImGui::BeginChild(drawname.c_str(), ImVec2(0, slider_height), false, ImGuiWindowFlags_ChildWindow))
			{
				ImGui::TextColored(ImVec4{1.0f, 0.3f, 0.3f, 1.0f}, drawname.c_str());
				if(!is_active_draw)
				{
					imgui_helper_tooltip("This draw is inactive. This means that you are viewing unused data within the draw-list's GPU buffer, but this doesn't represent an object currently being drawn.");
				}
				ImGui::Text("Vertex Offset: %u", mloc.vertex_offset);
				ImGui::Text("Vertex Count:  %u", mloc.vertex_count);
				ImGui::Text("Index Offset:  %u", mloc.index_offset);
				ImGui::Text("Index Count:   %u", mloc.index_count);
				ImGui::Spacing();
				ImGui::Text("To see bound textures and transform, see Object %d within the render pass section", draw_id);
			}
			ImGui::EndChild();
		}
		else
		{
			ImGui::Text("There are no active draws.");
		}
	}

	mesh_renderer::render_pass_t::render_pass_t(tz::gl::renderer_handle compute_pass, tz::gl::resource_handle compute_draw_indirect_buffer, unsigned int total_textures)
	{
		// we have a draw buffer which we write into upon render.
		tz::gl::renderer_info rinfo;
		// todo: shaders
		rinfo.shader().set_shader(tz::gl::shader_stage::vertex, ImportedShaderSource(mesh, vertex));
		rinfo.shader().set_shader(tz::gl::shader_stage::fragment, ImportedShaderSource(mesh, fragment));
		rinfo.set_options({tz::gl::renderer_option::draw_indirect_count});
		// index buffer (initially contains a single index. empty buffers should be a thing aaaa)
		this->index_buffer = rinfo.add_resource(tz::gl::buffer_resource::from_one
		(
			std::byte{0},
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

		std::size_t vertex_count = 0;
		std::size_t index_count = 0;
		for(const mesh_locator& loc : this->meshes)
		{
			vertex_count += loc.vertex_count;
			index_count += loc.index_count;
		}

		std::size_t vertex_capacity = ren.get_resource(this->vertex_buffer)->data_as<const mesh_renderer::vertex_t>().size();
		std::size_t index_capacity = ren.get_resource(this->index_buffer)->data_as<const index>().size();

		ImGui::Text("Vertex Usage: %zu/%zu (%zub/%zub - %.2f%%)", vertex_count, vertex_capacity, vertex_count * sizeof(mesh_renderer::vertex_t), vertex_capacity * sizeof(mesh_renderer::vertex_t), (vertex_count * 100.0f) / vertex_capacity);
		ImGui::Text("Index Usage:  %zu/%zu (%zub/%zub - %.2f%%)", index_count, index_capacity, index_count * sizeof(index), index_capacity * sizeof(index), (index_count * 100.0f) / index_capacity);

		static int mesh_locator_id = 0;
		if(this->meshes.size())
		{
			constexpr float slider_height = 160.0f;
			ImGui::VSliderInt("##mesh_locator", ImVec2{18.0f, slider_height}, &mesh_locator_id, 0, this->meshes.size() - 1);
			std::string locname = "Mesh Locator " + std::to_string(mesh_locator_id);
			ImGui::SameLine();

			// add slight horizontal spacing between slider and child.
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
			const mesh_locator& mloc = this->meshes[mesh_locator_id];
			if(ImGui::BeginChild(locname.c_str(), ImVec2(0, slider_height), false, ImGuiWindowFlags_ChildWindow))
			{
				ImGui::TextColored(ImVec4{1.0f, 0.3f, 0.3f, 1.0f}, locname.c_str());
				ImGui::Separator();
				ImGui::Text("Vertex Offset: %u", mloc.vertex_offset);
				ImGui::Text("Vertex Count:  %u", mloc.vertex_count);
				ImGui::Text("Index Offset:  %u", mloc.index_offset);
				ImGui::Text("Index Count:   %u", mloc.index_count);
			}
			ImGui::EndChild();
		}

		ImGui::Separator();
		ImGui::TextColored(ImVec4{1.0f, 0.3f, 0.3f, 1.0f}, "OBJECT DATA");
		imgui_helper_tooltip("An object represents a drawable element. It consists of a model matrix and a set of bound textures. Drawables are obviously associated with a mesh. Specifically, `object X` will use the mesh locator at element `X` of the draw-list. The draw-list is visible within the compute pass section, not here.");
		std::size_t object_count = ren.get_resource(this->object_buffer)->data_as<const object_data>().size();
		static int object_id = 0;
		if(object_count > 0)
		{
			constexpr float slider_height = 160.0f;
			ImGui::VSliderInt("##object", ImVec2{18.0f, slider_height}, &object_id, 0, object_count - 1);
			// TODO: object information display
			std::string objname = "Object " + std::to_string(object_id);
			ImGui::SameLine();
			// add slight horizontal spacing between slider and child.
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
			object_data& obj = ren.get_resource(this->object_buffer)->data_as<object_data>()[object_id];
			if(ImGui::BeginChild(objname.c_str(), ImVec2(0, slider_height), false, ImGuiWindowFlags_ChildWindow))
			{
				ImGui::TextColored(ImVec4{1.0f, 0.3f, 0.3f, 1.0f}, objname.c_str());
				std::string parent_str = std::to_string(obj.parent);
				if(obj.parent == static_cast<std::uint32_t>(-1))
				{
					parent_str = "none";
				}
				ImGui::Text("Parent: %s", parent_str.c_str());
				ImGui::Separator();
				ImGui::Text("Model Matrix");
				for (int row = 0; row < 4; row++)
				{
					for (int col = 0; col < 4; col++)
					{
						std::string label = "##m" + std::to_string(row) + std::to_string(col);
						constexpr float matrix_cell_width = 35.0f;
						ImGui::SetNextItemWidth(matrix_cell_width);
						ImGui::InputFloat(label.c_str(), &obj.model(row, col));
						ImGui::SameLine();
					}
					ImGui::NewLine();
				}

				ImGui::Separator();
				ImGui::Text("Textures");
				for(std::size_t i = 0; i < mesh_renderer_max_tex_count; i++)
				{
					ImGui::Text("[%zu] =", i);
					ImGui::SameLine();
					if(obj.bound_textures[i].is_null())
					{
						ImGui::TextColored({1.0f, 0.0f, 0.0f, 1.0f}, "unbound");
					}
					else
					{
						ImGui::Text("id %u", static_cast<unsigned int>(static_cast<tz::hanval>(obj.bound_textures[i].texture)));
					}
				}

				ImGui::Separator();
				ImGui::Text("Model");
				imgui_helper_tooltip("The list of object data does not contain info on which mesh it corresponds to. To do that, view the corresponding element of the draw list within the compute pass.");
			}
			ImGui::EndChild();
		}

		ImGui::Separator();
		ImGui::TextColored(ImVec4{1.0f, 0.3f, 0.3f, 1.0f}, "CAMERA DATA");
		auto& cam = ren.get_resource(this->camera_buffer)->data_as<camera_data>().front();
		ImGui::Text("View Matrix");
		tz::dbgui_view(cam.view);

		ImGui::Text("Projection Matrix");
		for (int row = 0; row < 4; row++)
		{
			for (int col = 0; col < 4; col++)
			{
				std::string label = "##p" + std::to_string(row) + std::to_string(col);
				constexpr float matrix_cell_width = 35.0f;
				ImGui::SetNextItemWidth(matrix_cell_width);
				ImGui::InputFloat(label.c_str(), &cam.projection(row, col));
				ImGui::SameLine();
			}
			ImGui::NewLine();
		}

		ImGui::Separator();
		ImGui::TextColored(ImVec4{1.0f, 0.3f, 0.3f, 1.0f}, "TEXTURES");
		std::size_t texture_count = this->texture_cursor;
		static int texture_id = 0;
		if(texture_count > 0)
		{
			constexpr float slider_height = 160.0f;
			ImGui::VSliderInt("##texture", ImVec2{18.0f, slider_height}, &texture_id, 0, texture_count - 1);
			// TODO: object information display
			std::string texname = "Texture " + std::to_string(texture_id);
			ImGui::SameLine();
			// add slight horizontal spacing between slider and child.
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
			const tz::gl::image_resource& img = *static_cast<tz::gl::image_resource*>(ren.get_resource(this->textures[texture_id]));
			if(ImGui::BeginChild(texname.c_str(), ImVec2(0, slider_height), false, ImGuiWindowFlags_ChildWindow))
			{
				ImGui::TextColored(ImVec4{1.0f, 0.3f, 0.3f, 1.0f}, texname.c_str());
				ImGui::Text("Dimensions: %ux%u", img.get_dimensions()[0], img.get_dimensions()[1]);
				ImGui::Text("Size: %zub", img.data().size_bytes());
			}
			ImGui::EndChild();
		}
		else
		{
			ImGui::Text("There are no textures.");
		}
	}

	std::span<const object_data> mesh_renderer::render_pass_t::get_object_datas() const
	{
		return tz::gl::get_device().get_renderer(this->handle).get_resource(this->object_buffer)->data_as<const object_data>();
	}

	std::span<object_data> mesh_renderer::render_pass_t::get_object_datas()
	{
		return tz::gl::get_device().get_renderer(this->handle).get_resource(this->object_buffer)->data_as<object_data>();
	}

	std::optional<std::uint32_t> mesh_renderer::try_find_index_section(std::size_t index_count) const
	{
		// Sort mesh locators by vertex offset
        std::vector<mesh_locator> sorted_meshes = this->render_pass.meshes;
        std::sort(sorted_meshes.begin(), sorted_meshes.end(),
                  [](const mesh_locator& a, const mesh_locator& b) {
                      return a.index_offset < b.index_offset;
                  });

        // Iterate through sorted mesh locators to find gaps
        std::uint32_t current_offset = 0;
        for (const mesh_locator &locator : sorted_meshes)
        {
            std::uint32_t gap_size = locator.index_offset - current_offset;
            if (gap_size >= index_count)
            {
                // Found a gap large enough
                return current_offset;
            }
            current_offset = locator.index_offset + locator.index_count;
        }

        // Check for space at the end of the buffer
        std::uint32_t last_mesh_end = sorted_meshes.empty() ? 0 : sorted_meshes.back().index_offset + sorted_meshes.back().index_count;
		std::size_t total_index_count = tz::gl::get_device().get_renderer(this->render_pass.handle).get_resource(this->render_pass.index_buffer)->data_as<index>().size();
        if (total_index_count - last_mesh_end >= index_count)
        {
            return last_mesh_end;
        }

        // No suitable gap found
		return std::nullopt;
	}

	std::optional<std::uint32_t> mesh_renderer::try_find_vertex_section(std::size_t vertex_count) const
	{
		// Sort mesh locators by vertex offset
        std::vector<mesh_locator> sorted_meshes = this->render_pass.meshes;
        std::sort(sorted_meshes.begin(), sorted_meshes.end(),
                  [](const mesh_locator& a, const mesh_locator& b) {
                      return a.vertex_offset < b.vertex_offset;
                  });

        // Iterate through sorted mesh locators to find gaps
        std::uint32_t current_offset = 0;
        for (const mesh_locator &locator : sorted_meshes)
        {
            std::uint32_t gap_size = locator.vertex_offset - current_offset;
            if (gap_size >= vertex_count)
            {
                // Found a gap large enough
                return current_offset;
            }
            current_offset = locator.vertex_offset + locator.vertex_count;
        }

        // Check for space at the end of the buffer
        std::uint32_t last_mesh_end = sorted_meshes.empty() ? 0 : sorted_meshes.back().vertex_offset + sorted_meshes.back().vertex_count;
		std::size_t total_vertex_count = tz::gl::get_device().get_renderer(this->render_pass.handle).get_resource(this->render_pass.vertex_buffer)->data_as<mesh_renderer::vertex_t>().size();
        if (total_vertex_count - last_mesh_end >= vertex_count)
        {
            return last_mesh_end;
        }

        // No suitable gap found
		return std::nullopt;
	}

	mesh_locator mesh_renderer::add_mesh_impl(const mesh_renderer::mesh_t& m)
	{
		auto& ren = tz::gl::get_device().get_renderer(this->render_pass.handle);
		tz::gl::RendererEditBuilder edit;

		std::span<const index> index_src = m.indices;
		std::span<const vertex_t> vertex_src = m.vertices;

		// let's check if our vertex and index buffers currently have enough free capacity to store the mesh.
		auto maybe_index_section = this->try_find_index_section(m.indices.size());
		if(maybe_index_section.has_value())
		{
			// write the index data into the spot.
			edit.write
			({
				.resource = this->render_pass.index_buffer,
				.data = std::as_bytes(index_src),
				.offset = maybe_index_section.value() * sizeof(index)
			});
		}
		else
		{
			// resize to make it large enough.
			edit.buffer_resize
			({
				.buffer_handle = this->render_pass.index_buffer,
				.size = ren.get_resource(this->render_pass.index_buffer)->data().size_bytes() + index_src.size_bytes()
			});
			// try again
			ren.edit(edit.build());
			return add_mesh_impl(m);
		}

		auto maybe_vertex_section = this->try_find_vertex_section(m.vertices.size());
		if(maybe_vertex_section.has_value())
		{
			// write the vertex data.
			edit.write
			({
				.resource = this->render_pass.vertex_buffer,
				.data = std::as_bytes(vertex_src),
				.offset = maybe_vertex_section.value() * sizeof(vertex_t)
			});
		}
		else
		{
			// resize to make it large enough.
			edit.buffer_resize
			({
				.buffer_handle = this->render_pass.vertex_buffer,
				.size = ren.get_resource(this->render_pass.vertex_buffer)->data().size_bytes() + vertex_src.size_bytes()
			});
			// try again
			ren.edit(edit.build());
			return add_mesh_impl(m);
		}

		ren.edit(edit.build());
		std::uint32_t vertex_offset = this->render_pass.cumulative_vertex_count;
		this->render_pass.cumulative_vertex_count += m.vertices.size();

		return
		{
			.vertex_offset = maybe_vertex_section.value(),
			.vertex_count = static_cast<std::uint32_t>(vertex_src.size()),
			.index_offset = maybe_index_section.value(),
			.index_count = static_cast<std::uint32_t>(index_src.size()),
			.max_index_value = vertex_offset
		};
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
		if(ImGui::BeginTabBar("#123"))
		{
			if(ImGui::BeginTabItem("Overview"))
			{
				std::size_t vertex_count = 0;
				std::size_t index_count = 0;
				for(const mesh_locator& loc : this->render_pass.meshes)
				{
					vertex_count += loc.vertex_count;
					index_count += loc.index_count;
				}
				ImGui::Text("Draw Count:     %zu", this->draw_count());
				ImGui::Text("Meshes Stored:  %zu", this->mesh_count());
				ImGui::Text("Total Indices:  %zu", index_count);
				ImGui::Text("Total Vertices: %zu", vertex_count);
				if(ImGui::Button("Add empty mesh"))
				{
					this->add_mesh({});
				}
				if(this->render_pass.meshes.size())
				{
					if(ImGui::Button("Add first-mesh drawable"))
					{
						this->add_object(static_cast<tz::hanval>(0));
					}
					imgui_helper_tooltip("Press this to add a drawable with all the defaults, using the first mesh.");
				}
				if(ImGui::Button("Remove all drawables"))
				{
					this->clear_draws();
				}
				imgui_helper_tooltip("Press this to clear all drawable objects. This means nothing is drawn, but all the mesh data is untouched.");
				if(ImGui::Button("Remove All"))
				{
					this->clear();
				}
				imgui_helper_tooltip("Press this to clear everything, removing all objects and all mesh data.");
				ImGui::EndTabItem();

			}
			if(ImGui::BeginTabItem("Render"))
			{
				if(ImGui::BeginTabBar("Render or Compute"))
				{
					if(ImGui::BeginTabItem("Compute Pass"))
					{
						this->compute_pass.dbgui();
						ImGui::EndTabItem();
					}
					if(ImGui::BeginTabItem("Render Pass"))
					{
						this->render_pass.dbgui();
						ImGui::EndTabItem();
					}
					ImGui::EndTabBar();
				}
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
}