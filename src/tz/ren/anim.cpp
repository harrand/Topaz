#include "tz/ren/anim.hpp"
#include "tz/gl/api/schedule.hpp"
#include "tz/gl/device.hpp"
#include "tz/gl/draw.hpp"
#include "tz/gl/resource.hpp"
#include "tz/gl/imported_shaders.hpp"
#include "tz/core/matrix.hpp"
#include "tz/core/data/trs.hpp"
#include "tz/dbgui/dbgui.hpp"
#include "tz/io/gltf.hpp"
#include "tz/core/matrix_transform.hpp"
#include "tz/core/job/job.hpp"
#include "tz/core/profile.hpp"
#include <filesystem>

#include ImportedShaderHeader(anim, compute)
#include ImportedShaderHeader(anim, vertex)
#include ImportedShaderHeader(anim, fragment)

namespace tz::ren
{
	/* anim_renderer implementation details
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

	// a draw list contains all the meshes that need to be drawn in the right order.
	// the compute pass will take this list and populate the draw-indirect buffer,
	// which the renderer will the use to draw everything.
	constexpr std::size_t initial_draw_capacity = 64;
	struct draw_list
	{
		std::uint32_t draw_count = 0;
		std::array<mesh_locator, initial_draw_capacity> meshes;
	};

	// represents the data of the camera.
	struct camera_data
	{
		tz::mat4 view = tz::view({0.0f, 0.0f, 5.0f}, {});
		tz::mat4 projection = tz::perspective(3.14159f * 0.5f, 1920.0f/1080.0f, 0.1f, 1000.0f);
	};

	anim_renderer::anim_renderer(unsigned int total_textures):
	compute_pass(),
	render_pass(this->compute_pass.handle, this->compute_pass.draw_indirect_buffer, total_textures)
	{
		this->add_mesh({});
	}

	anim_renderer::mesh_handle empty_mesh()
	{
		return static_cast<tz::hanval>(0);
	}

	anim_renderer::mesh_handle anim_renderer::add_mesh(anim_renderer::mesh_t m)
	{
		TZ_PROFZONE("Anim Renderer - Add Mesh", 0xFF44DD44);
		std::size_t hanval = this->render_pass.meshes.size();
		mesh_locator locator = this->add_mesh_impl(m);
		this->render_pass.meshes.push_back(locator);
		return static_cast<tz::hanval>(hanval);
	}

	anim_renderer::object_handle anim_renderer::add_object(mesh_handle m, object_data data, object_impl_data impl)
	{
		TZ_PROFZONE("Anim Renderer - Add Object", 0xFF44DD44);
		std::size_t hanval = this->draw_count();
		auto mesh_id = static_cast<std::size_t>(static_cast<tz::hanval>(m));
		// draw list at this position is now equal to the associated mesh_locator.
		const std::size_t object_capacity = this->render_pass.object_list_capacity();
		if(hanval >= object_capacity)
		{
			this->expand_object_capacity(object_capacity);
		}
		tz::assert(hanval < this->compute_pass.get_draw_list_meshes().size(), "ran out of objects! can only have %zu", this->compute_pass.get_draw_list_meshes().size());
		this->compute_pass.get_draw_list_meshes()[hanval] = this->render_pass.meshes[mesh_id];
		// now need to fill the object data
		this->render_pass.get_object_datas()[hanval] = data;
		this->object_impls.push_back(impl);
		// finally, increment the draw count.
		this->compute_pass.set_draw_count(this->compute_pass.get_draw_count() + 1);

		return static_cast<tz::hanval>(hanval);
	}

	anim_renderer::texture_handle anim_renderer::add_texture(tz::vec2ui dimensions, std::span<const std::byte> image_data)
	{
		TZ_PROFZONE("Anim Renderer - Add Texture", 0xFF44DD44);
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

	anim_renderer::stored_assets anim_renderer::add_gltf(const tz::io::gltf& gltf)
	{
		TZ_PROFZONE("Anim Renderer - Add GLTF", 0xFF44DD44);
		this->animation.gltfs.push_back(gltf);
		return this->add_gltf_impl(gltf);
	}

	std::size_t anim_renderer::mesh_count() const
	{
		return this->render_pass.meshes.size();
	}

	std::size_t anim_renderer::draw_count() const
	{
		return this->compute_pass.get_draw_count();
	}

	void anim_renderer::clear()
	{
		// does not change capacities of any buffers.
		this->clear_draws();
		// next mesh to be added will simply be right at the front and begin overwriting.
		this->render_pass.meshes.clear();
		this->render_pass.cumulative_vertex_count = 0;
		this->render_pass.texture_cursor = 0;
	}

	void anim_renderer::clear_draws()
	{
		this->compute_pass.set_draw_count(0);
	}

	void anim_renderer::append_to_render_graph()
	{
		auto hanval_ch = static_cast<tz::gl::eid_t>(static_cast<tz::hanval>(this->compute_pass.handle));
		auto hanval_rh = static_cast<tz::gl::eid_t>(static_cast<tz::hanval>(this->render_pass.handle));
		// compute pass happens first, then the render pass.
		tz::gl::get_device().render_graph().timeline.push_back(hanval_ch);
		tz::gl::get_device().render_graph().timeline.push_back(hanval_rh);
		// render pass depends on compute pass.
		tz::gl::get_device().render_graph().add_dependencies(this->render_pass.handle, this->compute_pass.handle);
	}

	void anim_renderer::update(float dt)
	{
		TZ_PROFZONE("Anim Renderer - Update", 0xFF44DD44);
		this->update_animated_nodes(dt);
		this->compute_global_transforms();
	}

	void anim_renderer::dbgui()
	{
		TZ_PROFZONE("Anim Renderer - Dbgui", 0xFF44DD44);
		this->dbgui_impl();
	}

	anim_renderer::compute_pass_t::compute_pass_t()
	{
		TZ_PROFZONE("Anim Renderer - Create Compute Pass", 0xFF44DD44);
		tz::gl::renderer_info cinfo;
		cinfo.shader().set_shader(tz::gl::shader_stage::compute, ImportedShaderSource(anim, compute));
		struct draw_indirect_buffer_data
		{
			std::uint32_t count;
			std::array<tz::gl::draw_indexed_indirect_command, initial_draw_capacity> cmds;
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
		cinfo.debug_name("Anim Renderer - Compute Pass");
		this->handle = tz::gl::get_device().create_renderer(cinfo);
	}

	std::span<const mesh_locator> anim_renderer::compute_pass_t::get_draw_list_meshes() const
	{
		return tz::gl::get_device().get_renderer(this->handle).get_resource(this->draw_list_buffer)->data_as<const draw_list>().front().meshes;
	}

	std::span<mesh_locator> anim_renderer::compute_pass_t::get_draw_list_meshes()
	{
		std::span<std::byte> data = tz::gl::get_device().get_renderer(this->handle).get_resource(this->draw_list_buffer)->data();
		// draw list starts with a uint32_t, rest are draw data.
		data = data.subspan(sizeof(std::uint32_t));
		return {reinterpret_cast<mesh_locator*>(data.data()), data.size_bytes() / sizeof(mesh_locator)};
	}

	std::uint32_t anim_renderer::compute_pass_t::get_draw_count() const
	{
		return tz::gl::get_device().get_renderer(this->handle).get_resource(this->draw_list_buffer)->data_as<draw_list>().front().draw_count;
	}

	void anim_renderer::compute_pass_t::set_draw_count(std::uint32_t new_draw_count)
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

	void anim_renderer::compute_pass_t::dbgui()
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

	void anim_renderer::compute_pass_t::increase_draw_list_capacity(std::size_t count)
	{
		TZ_PROFZONE("Anim Renderer - Draw List Append", 0xFF44DD44);
		auto& ren = tz::gl::get_device().get_renderer(this->handle);
		tz::gl::RendererEditBuilder builder{};
		{
			std::size_t byte_count = sizeof(mesh_locator) * count;
			std::size_t cursor = ren.get_resource(this->draw_list_buffer)->data().size_bytes();
			builder.buffer_resize
			({
				.buffer_handle = this->draw_list_buffer,
				.size = cursor + byte_count
			});
		}

		// finally, the draw indirect buffer
		// note we have to do it here, not on the compute renderer, as it has side effects and we will need to rerecord work commands.
		{
			std::size_t byte_count = sizeof(tz::gl::draw_indexed_indirect_command) * count;
			std::size_t cursor = ren.get_resource(this->draw_indirect_buffer)->data().size_bytes();
			builder.buffer_resize
			({
				.buffer_handle = this->draw_indirect_buffer,
				.size = cursor + byte_count
			});
		}

		ren.edit(builder.build());
	}

	anim_renderer::render_pass_t::render_pass_t(tz::gl::renderer_handle compute_pass, tz::gl::resource_handle compute_draw_indirect_buffer, unsigned int total_textures)
	{
		TZ_PROFZONE("Anim Renderer - Render Pass Create", 0xFF44DD44);
		// we have a draw buffer which we write into upon render.
		tz::gl::renderer_info rinfo;
		// todo: shaders
		rinfo.shader().set_shader(tz::gl::shader_stage::vertex, ImportedShaderSource(anim, vertex));
		rinfo.shader().set_shader(tz::gl::shader_stage::fragment, ImportedShaderSource(anim, fragment));
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
		std::array<object_data, initial_draw_capacity> initial_object_data = {};
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
		std::array<std::uint32_t, initial_draw_capacity> itoib_data{};
		std::fill(itoib_data.begin(), itoib_data.end(), static_cast<std::uint32_t>(-1));
		this->joint_id_to_node_index = rinfo.add_resource(tz::gl::buffer_resource::from_one
		(
			itoib_data,
			{
				.access = tz::gl::resource_access::dynamic_access
			}
		));
		this->index_to_object_id_buffer = rinfo.add_resource(tz::gl::buffer_resource::from_one
		(
			itoib_data,
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
		rinfo.debug_name("Anim Renderer - Render Pass");
		this->handle = tz::gl::get_device().create_renderer(rinfo);
	}

	void anim_renderer::render_pass_t::dbgui()
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

		std::size_t vertex_capacity = ren.get_resource(this->vertex_buffer)->data_as<const anim_renderer::vertex_t>().size();
		std::size_t index_capacity = ren.get_resource(this->index_buffer)->data_as<const index>().size();

		ImGui::Text("Vertex Usage: %zu/%zu |", vertex_count, vertex_capacity); ImGui::SameLine();
		tz::dbgui::text_memory(vertex_count * sizeof(anim_renderer::vertex_t)); ImGui::SameLine();
		ImGui::Text("/"); ImGui::SameLine();
		tz::dbgui::text_memory(vertex_capacity * sizeof(anim_renderer::vertex_t)); ImGui::SameLine();
		ImGui::Text(" - %.2f%%", (vertex_count * 100.0f) / vertex_capacity);

		ImGui::Text("Index Usage: %zu/%zu |", index_count, index_capacity); ImGui::SameLine();
		tz::dbgui::text_memory(index_count * sizeof(index)); ImGui::SameLine();
		ImGui::Text("/"); ImGui::SameLine();
		tz::dbgui::text_memory(index_capacity * sizeof(index)); ImGui::SameLine();
		ImGui::Text(" - %.2f%%", (index_count * 100.0f) / index_capacity);

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
				ImGui::Text("Global Transform");
				tz::dbgui_model(obj.global_transform);

				ImGui::Separator();
				ImGui::Text("Textures");
				for(std::size_t i = 0; i < anim_renderer_max_tex_count; i++)
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
		ImGui::Spacing();

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

	std::span<const object_data> anim_renderer::render_pass_t::get_object_datas() const
	{
		return tz::gl::get_device().get_renderer(this->handle).get_resource(this->object_buffer)->data_as<const object_data>();
	}

	std::span<object_data> anim_renderer::render_pass_t::get_object_datas()
	{
		return tz::gl::get_device().get_renderer(this->handle).get_resource(this->object_buffer)->data_as<object_data>();
	}

	std::span<std::uint32_t> anim_renderer::render_pass_t::get_joint_id_to_node_ids()
	{
		return tz::gl::get_device().get_renderer(this->handle).get_resource(this->joint_id_to_node_index)->data_as<std::uint32_t>();
	}

	std::span<std::uint32_t> anim_renderer::render_pass_t::get_index_to_object_ids()
	{
		return tz::gl::get_device().get_renderer(this->handle).get_resource(this->index_to_object_id_buffer)->data_as<std::uint32_t>();
	}

	std::size_t anim_renderer::render_pass_t::object_list_capacity() const
	{
		return this->get_object_datas().size();
	}

	void anim_renderer::render_pass_t::increase_object_list_capacity(std::size_t count)
	{
		TZ_PROFZONE("Anim Renderer - Object List Append", 0xFF44DD44);
		tz::gl::RendererEditBuilder builder;
		std::size_t prev_object_count = 0;
		auto& ren = tz::gl::get_device().get_renderer(this->handle);
		{
			std::size_t byte_count = sizeof(object_data) * count;
			std::size_t cursor = ren.get_resource(this->object_buffer)->data().size_bytes();
			prev_object_count = cursor / sizeof(object_data);
			builder.buffer_resize
			({
				.buffer_handle = this->object_buffer,
				.size = cursor + byte_count
			});
		}

		// and id resolution buffers...
		{
			std::size_t byte_count = sizeof(std::uint32_t) * count;
			std::size_t cursor = ren.get_resource(this->joint_id_to_node_index)->data().size_bytes();
			builder.buffer_resize
			({
				.buffer_handle = this->joint_id_to_node_index,
				.size = cursor + byte_count
			});
		}
		
		{
			std::size_t byte_count = sizeof(std::uint32_t) * count;
			std::size_t cursor = ren.get_resource(this->index_to_object_id_buffer)->data().size_bytes();
			builder.buffer_resize
			({
				.buffer_handle = this->index_to_object_id_buffer,
				.size = cursor + byte_count
			});
		}

		builder.mark_dirty({});

		ren.edit(builder.build());

		// we also iterate through all objects, so we can't have garbage data here.
		for(std::size_t i = 0; i < count; i++)
		{
			this->get_object_datas()[i + prev_object_count] = object_data{};
		}
	}

	void anim_renderer::expand_object_capacity(std::size_t extra_count)
	{
		TZ_PROFZONE("Anim Renderer - Expand Capacity", 0xFF44DD44);
		// we ran out of space.
		// resize compute's draw list and render's object buffer.
		// add new slots equal to current count (i.e double capacity)
		#if TZ_DEBUG
		const std::size_t object_capacity = this->render_pass.object_list_capacity();
		tz::report("expanding object capacity %zu => %zu", object_capacity, object_capacity + extra_count);
		#endif // TZ_DEBUG
		this->compute_pass.increase_draw_list_capacity(extra_count);
		this->render_pass.increase_object_list_capacity(extra_count);

	}

	std::optional<std::uint32_t> anim_renderer::try_find_index_section(std::size_t index_count) const
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

	std::optional<std::uint32_t> anim_renderer::try_find_vertex_section(std::size_t vertex_count) const
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
		std::size_t total_vertex_count = tz::gl::get_device().get_renderer(this->render_pass.handle).get_resource(this->render_pass.vertex_buffer)->data_as<anim_renderer::vertex_t>().size();
        if (total_vertex_count - last_mesh_end >= vertex_count)
        {
            return last_mesh_end;
        }

        // No suitable gap found
		return std::nullopt;
	}

	mesh_locator anim_renderer::add_mesh_impl(const anim_renderer::mesh_t& m)
	{
		if(m.indices.empty() && m.vertices.empty())
		{
			return 
			{
				.vertex_offset = 0,
				.vertex_count = 0,
				.index_offset = 0,
				.index_count = 0,
				.max_index_value = static_cast<std::uint32_t>(this->render_pass.cumulative_vertex_count)
			};
		}
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

	void anim_renderer::dbgui_impl()
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
				
				ImGui::Separator();

				// Add ability to import meshes at runtime. How cool.
				ImGui::Text("Import GLTF file");
				static std::string mesh_path = "../abc.glb";
				static bool failed_to_load = false;
				static std::string failure_str = "No error";
				ImGui::InputText("Path to GLB", &mesh_path);
				if(ImGui::Button("Add to scene"))
				{
					if(std::filesystem::exists(mesh_path))
					{
						this->add_gltf(tz::io::gltf::from_file(mesh_path.c_str()));
						failed_to_load = false;
					}
					else
					{
						failed_to_load = true;
						failure_str = "No file exists at the path \"" + mesh_path + "\"";
					}
				}
				if(failed_to_load)
				{
					ImGui::Text("Failed to Load: %s", failure_str.c_str());
				}
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
			if(ImGui::BeginTabItem("Animation"))
			{
				this->dbgui_anim();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}

	anim_renderer::stored_assets anim_renderer::add_gltf_impl(const tz::io::gltf& gltf)
	{
		anim_renderer::stored_assets ret;
		// firstly, add all the meshes in the scene.
		// todo: only add meshes referenced by the nodes?
		// each mesh contains zero or more submeshes. for mesh i, gltf_mesh_index_begin[i] represents the index into the flatenned submesh array that the first submesh begins.
		std::vector<std::size_t> gltf_mesh_index_begin = {};
		std::vector<std::optional<tz::io::gltf_material>> gltf_submesh_bound_textures = {};
		std::size_t gltf_submesh_total = 0;
		for(std::size_t i = 0; i < gltf.get_meshes().size(); i++)
		{
			TZ_PROFZONE("Anim Renderer - Add GLTF Mesh", 0xFF44DD44);
			std::size_t submesh_count = gltf.get_meshes()[i].submeshes.size();
			gltf_mesh_index_begin.push_back(gltf_submesh_total);
			gltf_submesh_total += submesh_count;
			for(std::size_t j = 0; j < submesh_count; j++)
			{
				mesh_t submesh;

				std::size_t material_id = gltf.get_meshes()[i].submeshes[j].material_id;
				auto& maybe_material = gltf_submesh_bound_textures.emplace_back();
				if(material_id != static_cast<std::size_t>(-1))
				{
					// no material bound, so no textures bound.
					maybe_material = gltf.get_materials()[material_id];
				}
				tz::io::gltf_submesh_data gltf_submesh = gltf.get_submesh_vertex_data(i, j);
				// copy over indices.
				submesh.indices.resize(gltf_submesh.indices.size());
				std::transform(gltf_submesh.indices.begin(), gltf_submesh.indices.end(), submesh.indices.begin(),
				[](std::uint32_t idx)-> index{return idx;});
				// copy over vertices.
				std::transform(gltf_submesh.vertices.begin(), gltf_submesh.vertices.end(), std::back_inserter(submesh.vertices),
				[this](tz::io::gltf_vertex_data vtx)-> vertex_t
				{
					vertex_t ret;

					// these are easy.
					ret.position = vtx.position;
					ret.normal = vtx.normal;
					ret.tangent = vtx.tangent;

					// texcoord a little more troublesome!
					constexpr std::size_t texcoord_count = std::min(static_cast<int>(anim_renderer_max_tex_count), tz::io::gltf_max_texcoord_attribs);
					for(std::size_t i = 0; i < texcoord_count; i++)
					{
						ret.texcoordn[i] = vtx.texcoordn[i].with_more(0.0f).with_more(0.0f);
					}

					// similar with weights.
					constexpr std::size_t weight_count = std::min(static_cast<int>(anim_renderer_max_joint4_count), tz::io::gltf_max_joint_attribs);
					for(std::size_t i = 0; i < weight_count; i++)
					{
						ret.joint_indices[i] = vtx.jointn[i] + tz::vec4us::filled(this->get_gltf_node_offset());
						ret.joint_weights[i] = vtx.weightn[i];
					}
					// ignore colours. could theoretically incorporate that into tints, but will be very difficult to translate to texture locator tints properly.
					return ret;
				});
				// add the mesh!
				ret.meshes.push_back(this->add_mesh(submesh));
			}
		}

		// now, textures
		std::vector<tz::io::image> images;
		images.resize(gltf.get_images().size());
		if(gltf.get_images().size() > 8)
		{
			TZ_PROFZONE("Load Images - Jobs", 0xFF44DD44);
			// we should split this into threads.
			std::size_t job_count = std::thread::hardware_concurrency();
			std::size_t imgs_per_job = gltf.get_images().size() / job_count; 
			std::size_t remainder_imgs = gltf.get_images().size() % job_count;
			std::vector<tz::job_handle> jobs(job_count);
			for(std::size_t i = 0; i < job_count; i++)
			{
				jobs[i] = tz::job_system().execute([&images, &gltf, offset = i * imgs_per_job, img_count = imgs_per_job]()
				{
					for(std::size_t j = 0; j < img_count; j++)
					{
						images[offset + j] = gltf.get_image_data(offset + j);
					}
				});
			}
			for(std::size_t i = (gltf.get_images().size() - remainder_imgs); i < gltf.get_images().size(); i++)
			{
				images[i] = gltf.get_image_data(i);
			}
			for(tz::job_handle jh : jobs)
			{
				tz::job_system().block(jh);
			}
		}
		else
		{
			TZ_PROFZONE("Load Images - Single Threaded", 0xFF44DD44);
			// if there isn't many, just do it all now.
			for(std::size_t i = 0; i < gltf.get_images().size(); i++)
			{
				images[i] = gltf.get_image_data(i);
			}
		}
		for(const tz::io::image& img : images)
		{
			ret.textures.push_back(this->add_texture(tz::vec2ui{img.width, img.height}, img.data));
		}

		// finally, objects (aka nodes)
		std::vector<tz::io::gltf_node> nodes = gltf.get_active_nodes();
		for(tz::io::gltf_node active_node : nodes)
		{
			// add this node's submeshes as objects (recursively for children too).
			this->impl_expand_gltf_node(gltf, active_node, ret, gltf_mesh_index_begin, gltf_submesh_bound_textures);
		}
		std::size_t node_count = gltf.get_nodes().size();
		this->compute_global_transforms();
		this->process_skins();
		this->gltf_metas.push_back({.node_count = node_count});
		return ret;
	}

	void anim_renderer::impl_expand_gltf_node(const tz::io::gltf& gltf, const tz::io::gltf_node& node, anim_renderer::stored_assets& assets, std::span<std::size_t> mesh_submesh_indices, std::span<std::optional<tz::io::gltf_material>> submesh_materials, std::uint32_t parent)
	{
		TZ_PROFZONE("Anim Renderer - Expand GLTF Node", 0xFF44DD44);
		auto nodes = gltf.get_nodes();
		auto iter = std::find(nodes.begin(), nodes.end(), node);
		tz::assert(iter != nodes.end());
		std::uint32_t our_gltf_node_id = this->get_gltf_node_offset() + std::distance(nodes.begin(), iter);

		std::uint32_t our_object_id = this->draw_count();

		const std::size_t object_capacity = this->render_pass.object_list_capacity();
		if(our_gltf_node_id >= object_capacity)
		{
			this->expand_object_capacity(object_capacity);
		}
		this->render_pass.get_index_to_object_ids()[our_gltf_node_id] = our_object_id;
		// a node might have a mesh attached. remember, a gltf mesh is a set of submeshes, so we want multiple objects per node.
		// number of objects per node = number of submeshes within the node's mesh, if it has one. recurse for its children.

		// however, if a node has no mesh attached, we add an object for it anyway. that object is a zero draw (i know, that kinda sucks for efficiency).
		// this is so we can fully respect the transform hierarchy - a bone for example might not have any drawable component, but is an important parent
		// for something that *is* drawable.

		// gltf spec says:
		// Only the joint transforms are applied to the skinned mesh; the transform of the skinned mesh node MUST be ignored.
		// this means, if a node has a skin, ignore its transform and use the transform of the skin (joints etc) instead.
		tz::mat4 transform = node.transform.matrix();
		if(node.skin != static_cast<std::size_t>(-1))
		{
			this->skins_to_process.push_back(gltf.get_skins()[node.skin]);
		}
		if(node.mesh != static_cast<std::size_t>(-1))
		{
			std::size_t submesh_count = gltf.get_meshes()[node.mesh].submeshes.size();
			std::size_t submesh_offset = mesh_submesh_indices[node.mesh];
			// add an empty object, and a bunch of children for each submesh.
			assets.objects.push_back(this->add_object(empty_mesh(), {.parent = parent}, {.model = transform}));
			for(std::size_t i = submesh_offset; i < (submesh_offset + submesh_count); i++)
			{
				std::array<texture_locator, anim_renderer_max_tex_count> bound_textures = {};
				if(submesh_materials[i].has_value())
				{
					bound_textures[0] =
					{
						.colour_tint = tz::vec3::filled(1.0f),
						.texture = assets.textures[submesh_materials[i]->color_texture_id]
					};
				}
				assets.objects.push_back(this->add_object(assets.meshes[i], {.bound_textures = bound_textures, .parent = our_object_id}));
			}
		}
		else
		{
			// add a single object referencing the null mesh, so we can still have it in the transform hierarchy.
			assets.objects.push_back(this->add_object(empty_mesh(), {.parent = parent}, {.model = transform}));
		}
		for(std::size_t child_idx : node.children)
		{
			this->impl_expand_gltf_node(gltf, gltf.get_nodes()[child_idx], assets, mesh_submesh_indices, submesh_materials, our_object_id);
		}
	}

	tz::mat4 anim_renderer::compute_global_transform(std::uint32_t obj_id, std::vector<std::uint32_t>& visited_node_ids) const
	{
		TZ_PROFZONE("Anim Renderer - Compute Global Transform", 0xFF44DD44);
		auto& obj = this->render_pass.get_object_datas()[obj_id];
		const auto& objimpl = this->object_impls[obj_id];
		if(std::find(visited_node_ids.begin(), visited_node_ids.end(), obj_id) != visited_node_ids.end())
		{
			return obj.global_transform;
		}
		tz::mat4 global = objimpl.anim_transform;
		if(obj.parent != static_cast<std::uint32_t>(-1))
		{
			global = compute_global_transform(obj.parent, visited_node_ids) * global;
		}
		return global;
	}

	void anim_renderer::compute_global_transforms()
	{
		TZ_PROFZONE("Anim Renderer - Compute All Transforms", 0xFF44DD44);
		auto objs = this->render_pass.get_object_datas();
		std::vector<std::uint32_t> visited_node_ids = {};
		// this can get pretty costly
		for(std::size_t i = 0; i < this->draw_count(); i++)
		{
			objs[i].global_transform = this->compute_global_transform(i, visited_node_ids);
			visited_node_ids.push_back(i);
		}
	}

	void anim_renderer::process_skins()
	{
		TZ_PROFZONE("Anim Renderer - Process Skins", 0xFF44DD44);
		for(const tz::io::gltf_skin& skin : this->skins_to_process)
		{
			for(std::size_t i = 0; i < skin.joints.size(); i++)
			{
				std::uint32_t joint_id = skin.joints[i] + this->get_gltf_node_offset();
				// remember, this is a gltf node id. we want the corresponding object.
				this->render_pass.get_joint_id_to_node_ids()[i + this->get_gltf_node_offset()] = joint_id;
				std::uint32_t obj_id = this->render_pass.get_index_to_object_ids()[joint_id];
				this->render_pass.get_object_datas()[obj_id].inverse_bind_matrix = skin.inverse_bind_matrices[i];
			}
		}
		this->skins_to_process.clear();
	}

	std::size_t anim_renderer::get_gltf_node_offset() const
	{
		if(this->gltf_metas.empty())
		{
			return 0;
		}
		return this->get_gltf_node_offset(this->gltf_metas.size() - 1);
	}

	std::size_t anim_renderer::get_gltf_node_offset(std::size_t gltf_cursor) const
	{
		std::size_t nc = 0;
		tz::assert(gltf_cursor < this->gltf_metas.size());
		for(std::size_t i = 0; i < gltf_cursor; i++)
		{
			nc += this->gltf_metas[i].node_count;
		}
		return nc;
	}

	std::pair<std::size_t, std::size_t> anim_renderer::gltf_animation_data::get_keyframe_indices_at(keyframe_iterator front, keyframe_iterator back) const
	{
		TZ_PROFZONE("Anim Renderer - Keyframe Pair Retrieve", 0xFF44DD44);
		keyframe_iterator iter = front;
		while(iter != back && iter->time_point <= this->time)
		{
			iter++;
		}
		std::size_t idx = std::distance(front, iter);
		if(idx == 0)
		{
			return {0u, 1u};
		}
		auto dist = std::distance(front, back);
		if(std::cmp_greater_equal(idx, dist))
		{
			return {dist - 2, dist - 1};
		}
		return {idx - 1, idx};
	}

	tz::vec4 quat_slerp(const tz::vec4& lhs, const tz::vec4& rhs, float interp)
	{
		TZ_PROFZONE("Anim Renderer - Quaternion Slerp", 0xFF44DD44);
		float cos_theta = lhs.dot(rhs);

		if (cos_theta < 0.0f) {
			// If the quaternions are in opposite directions, negate one to take the shortest path
			tz::vec4 neg_rhs = { -rhs[0], -rhs[1], -rhs[2], -rhs[3] };
			return quat_slerp(lhs, neg_rhs, interp);
		}

		const float threshold = 0.9995f;
		if (cos_theta > threshold) {
			// Linear interpolation for small angles
			tz::vec4 result = {
				lhs[0] + interp * (rhs[0] - lhs[0]),
				lhs[1] + interp * (rhs[1] - lhs[1]),
				lhs[2] + interp * (rhs[2] - lhs[2]),
				lhs[3] + interp * (rhs[3] - lhs[3])
			};
			return result.normalised();
		}

		float angle = std::acos(cos_theta);
		float sin_angle = std::sin(angle);
		float t0 = std::sin((1.0f - interp) * angle) / sin_angle;
		float t1 = std::sin(interp * angle) / sin_angle;

		tz::vec4 result = {
			t0 * lhs[0] + t1 * rhs[0],
			t0 * lhs[1] + t1 * rhs[1],
			t0 * lhs[2] + t1 * rhs[2],
			t0 * lhs[3] + t1 * rhs[3]
		};
		
		return result.normalised();
	}

	void anim_renderer::update_animated_nodes(float dt)
	{
		TZ_PROFZONE("Anim Renderer - Update Animations", 0xFF44DD44);
		this->animation.time += dt * this->animation.speed;
		if(this->animation.time < 0.0f)
		{
			this->animation.time = 99999.0f;
		}
		const tz::io::gltf& gltf = this->animation.gltfs[this->animation.gltf_cursor];
		for(const tz::io::gltf_animation& anim : gltf.get_animations())
		{
			if(this->animation.time > anim.max_time)
			{
				if(this->animation.speed >= 0.0f)
				{
					this->animation.time = 0.0f;
				}
				else
				{
					this->animation.time = anim.max_time;
				}
			}
			TZ_PROFZONE("Anim Renderer - Update Animation", 0xFF44DD44);
			for(std::size_t nid = 0; nid < anim.node_animation_data.size(); nid++)
			{
				TZ_PROFZONE("Anim Renderer - Animation Node Update", 0xFF44DD44);
				auto offset = this->get_gltf_node_offset(this->animation.gltf_cursor);
				std::uint32_t object_id = this->render_pass.get_index_to_object_ids()[nid + offset];
				object_impl_data& objimpl = this->object_impls[object_id];

				const auto& [kf_positions, kf_rotations, kf_scales] = anim.node_animation_data[nid];
				auto [pos_before_id, pos_after_id] = this->animation.get_keyframe_indices_at(kf_positions.begin(), kf_positions.end());
				auto [rot_before_id, rot_after_id] = this->animation.get_keyframe_indices_at(kf_rotations.begin(), kf_rotations.end());
				auto [scale_before_id, scale_after_id] = this->animation.get_keyframe_indices_at(kf_scales.begin(), kf_scales.end());
				tz::trs trs;
				bool no_transform = true;
				// pos
				if(kf_positions.size() > 1)
				{
					no_transform = false;
					auto before = kf_positions.begin();
					auto after = before;
					std::advance(before, pos_before_id);
					std::advance(after, pos_after_id);
					tz::assert(this->animation.time >= before->time_point);
					float pos_interp = std::clamp((this->animation.time - before->time_point) / (after->time_point - before->time_point), 0.0f, 1.0f);
					tz::vec3 beforet = before->transform.swizzle<0, 1, 2>();
					tz::vec3 aftert = after->transform.swizzle<0, 1, 2>();
					trs.translate = beforet + ((aftert - beforet) * pos_interp);
				}
				else if(kf_positions.size() == 1)
				{
					trs.translate = kf_positions.begin()->transform.swizzle<0, 1, 2>();
				}
				// rot
				if(kf_rotations.size() > 1)
				{
					no_transform = false;
					auto before = kf_rotations.begin();
					auto after = before;
					std::advance(before, rot_before_id);
					std::advance(after, rot_after_id);
					tz::assert(this->animation.time >= before->time_point);
					float rot_interp = std::clamp((this->animation.time - before->time_point) / (after->time_point - before->time_point), 0.0f, 1.0f);
					tz::quat beforer = before->transform;
					tz::quat afterr = after->transform;
					trs.rotate = beforer.slerp(afterr, rot_interp);
				}
				// scale
				if(kf_scales.size() > 1)
				{
					auto before = kf_scales.begin();
					auto after = before;
					std::advance(before, scale_before_id);
					std::advance(after, scale_after_id);
					tz::assert(this->animation.time >= before->time_point);
					float scale_interp = std::clamp((this->animation.time - before->time_point) / (after->time_point - before->time_point), 0.0f, 1.0f);
					tz::vec3 befores = before->transform.swizzle<0, 1, 2>();
					tz::vec3 afters = after->transform.swizzle<0, 1, 2>();
					trs.scale = befores + ((afters - befores) * scale_interp);
				}
				if(no_transform)
				{
					objimpl.anim_transform = objimpl.model;
				}
				else
				{
					objimpl.anim_transform = trs.matrix();
				}
			}
		}
	}

	void anim_renderer::dbgui_anim()
	{
		if(this->animation.gltfs.empty())
		{
			ImGui::Text("No animations present");
			return;
		}
		ImGui::SliderInt("GLTF ID", reinterpret_cast<int*>(&this->animation.gltf_cursor), 0, this->animation.gltfs.size() - 1);
		const auto& anims = this->animation.gltfs[this->animation.gltf_cursor].get_animations();
		const auto& anim = anims[this->animation.animation_cursor];
		if(ImGui::BeginCombo("Animation", anim.name.c_str()))
		{
			for(std::size_t i = 0; i < anims.size(); i++)
			{
				bool is_selected = (i == this->animation.animation_cursor);
				if(ImGui::Selectable(anims[i].name.c_str(), is_selected))
				{
					
				}
				if(is_selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		if(anim.max_time == 0)
		{
			return;
		}
		ImGui::Text("Progress (%.2fs/%.2fs)", this->animation.time, anim.max_time);
		ImGui::ProgressBar(this->animation.time / anim.max_time);
		ImGui::SliderFloat("Animation Speed", &this->animation.speed, -2.0f, 2.0f);
		if(ImGui::Button("Reset"))
		{
			this->animation.time = 0.0f;
		}
	}
}