#include "mesh_renderer.hpp"
#include "tz/core/profile.hpp"
#include "tz/gl/imported_shaders.hpp"
#include "tz/gl/resource.hpp"
#include "tz/gl/draw.hpp"
#include "tz/core/matrix_transform.hpp"
#include "tz/dbgui/dbgui.hpp"
#include "tz/wsi/keyboard.hpp"
#include "tz/wsi/mouse.hpp"

#include ImportedShaderHeader(mesh3d, vertex)
#include ImportedShaderHeader(mesh3d, fragment)
#include ImportedShaderHeader(preprocess, compute)

meshid_t mesh_renderer_entry::meshid_internal_count = 0;

mesh_renderer::mesh_renderer(std::size_t max_texture_count):
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
rh([this, max_texture_count]()
{
	this->textures.resize(max_texture_count);
	tz::gl::renderer_info rinfo;
	rinfo.shader().set_shader(tz::gl::shader_stage::vertex, ImportedShaderSource(mesh3d, vertex));
	rinfo.shader().set_shader(tz::gl::shader_stage::fragment, ImportedShaderSource(mesh3d, fragment));
	rinfo.set_options({tz::gl::renderer_option::draw_indirect_count});
	this->ib = rinfo.add_resource(tz::gl::buffer_resource::from_one(std::uint32_t{0},
	{
		.flags = {tz::gl::resource_flag::index_buffer}
	}));
	this->vb = rinfo.add_resource(tz::gl::buffer_resource::from_one(std::byte{0}));
	this->drawdata_buf = rinfo.add_resource(tz::gl::buffer_resource::from_one(drawdata_storage_t{},
	{
		.access = tz::gl::resource_access::dynamic_access
	}));
	this->camera_buf = rinfo.add_resource(tz::gl::buffer_resource::from_one(camera_data_t{},
	{
		.access = tz::gl::resource_access::dynamic_access
	}));
	this->dbref = rinfo.ref_resource(this->ch, this->db);
	for(std::size_t i = 0; i < max_texture_count; i++)
	{
		this->textures[i] = rinfo.add_resource(tz::gl::image_resource::from_uninitialised
		({
	   		.format = tz::gl::image_format::RGBA32,
			.dimensions = {1u, 1u},
	   		.flags = {tz::gl::resource_flag::image_wrap_repeat}
		}));
	}
	rinfo.state().graphics.draw_buffer = this->dbref;
	rinfo.state().graphics.index_buffer = this->ib;
	rinfo.debug_name("Mesh Renderer");
	return tz::gl::get_device().create_renderer(rinfo);
}())
{
	this->write_camera_buffer();
}

meshid_t mesh_renderer::add_mesh(mesh_t mesh, const char* name)
{
	this->entries.push_back
	({
		.vtx_count = static_cast<unsigned int>(mesh.vertices.size()),
	  	.idx_count = static_cast<unsigned int>(mesh.indices.size()),
	  	.max_idx = this->cumulative_vertex_count
	});
	this->cumulative_vertex_count += mesh.vertices.size();
	this->entry_names.push_back(name);
	this->append_mesh_to_buffers(mesh);
	return this->entries.back().meshid;
}

texid_t mesh_renderer::add_texture(unsigned int width, unsigned int height, std::span<const std::byte> imgdata)
{
	#if TZ_DEBUG
		std::size_t sz = tz::gl::pixel_size_bytes(tz::gl::image_format::RGBA32) * width * height;
		tz::assert(imgdata.size_bytes() == sz, "Unexpected image data length. Expected %zuB, but was %zuB", sz, imgdata.size_bytes());
	#endif
	auto& ren = tz::gl::get_device().get_renderer(this->rh);
	tz::assert(this->texture_cursor < this->textures.size(), "ran out of textures.");
	tz::gl::resource_handle imgh = this->textures[this->texture_cursor];
	ren.edit(tz::gl::RendererEditBuilder{}
	.image_resize({.image_handle = imgh, .dimensions = {width, height}})
	.build());
	ren.edit(tz::gl::RendererEditBuilder{}
	.write
	({
		.resource = imgh,
		.data = imgdata
	})
	.build());
	return this->texture_cursor++;
}

void mesh_renderer::push_back_timeline() const
{
	auto hanval_ch = static_cast<tz::gl::eid_t>(static_cast<tz::hanval>(this->ch));
	auto hanval_rh = static_cast<tz::gl::eid_t>(static_cast<tz::hanval>(this->rh));
	tz::gl::get_device().render_graph().timeline.push_back(hanval_ch);
	tz::gl::get_device().render_graph().timeline.push_back(hanval_rh);
	tz::gl::get_device().render_graph().add_dependencies(this->rh, this->ch);
}

void mesh_renderer::add_to_draw_list(meshid_t mesh, transform_t transform, std::vector<texid_t> tex)
{
	this->append_meshid_to_draw_buffer(mesh, transform, tex);
	this->draw_list.push_back(mesh);
	this->write_camera_buffer();
}

void mesh_renderer::dbgui()
{
	ImGui::Text("Mesh Count: %zu", this->entries.size());
	if(ImGui::BeginTabBar("#mesh_renderer"))
	{
		if(ImGui::BeginTabItem("Mesh Data"))
		{
			if(this->entries.size())
			{
				if(ImGui::CollapsingHeader("Total Statistics"))
				{
					std::size_t cum_vertices = 0;
					std::size_t cum_indices = 0;
					for(const auto& entry : this->entries)
					{
						cum_vertices += entry.vtx_count;
						cum_indices += entry.idx_count;
					}
					ImGui::Text("vertex count: %zu (%zuB)", cum_vertices, (cum_vertices * sizeof(vertex_t)));
					ImGui::Text("index count: %zu (%zuB)", cum_indices, (cum_indices * sizeof(std::uint32_t)));
				}
				static int mesh_view_id = 0;
				ImGui::SliderInt("Mesh Viewer", &mesh_view_id, 0, this->entries.size() - 1);
				mesh_view_id = std::min(static_cast<std::size_t>(mesh_view_id), this->entries.size() - 1);

				const mesh_renderer_entry& entry = this->entries[mesh_view_id];
				const char* name = this->entry_names[mesh_view_id];
				ImGui::Indent();
				ImGui::TextColored(ImVec4{1.0f, 0.6f, 0.6f, 1.0f}, "%s (%zu)", name, entry.meshid);
				ImGui::Text("vertex count: %zu (%zuB)", entry.vtx_count, (entry.vtx_count * sizeof(vertex_t)));
				ImGui::Text("index count: %zu (%zuB)", entry.idx_count, (entry.idx_count * sizeof(std::uint32_t)));
				ImGui::Unindent();
			}
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Renderable Objects"))
		{
			if(ImGui::BeginTabBar("#renderable objects"))
			{
				if(ImGui::BeginTabItem("Add/Remove Objects"))
				{
					static int create_meshid = this->entries.front().meshid;
					const char* create_meshname = this->entry_names.front();
					if(ImGui::BeginCombo("Mesh", create_meshname))
					{
						for(int n = 0; n < this->entries.size(); n++)
						{
							bool is_selected = (create_meshid == this->entries[n].meshid);
							if(ImGui::Selectable(this->entry_names[n], is_selected))
							{
								create_meshid = this->entries[n].meshid;
								create_meshname = this->entry_names[n];
							}
							if(is_selected)
							{
								ImGui::SetItemDefaultFocus();
							}
						}
						ImGui::EndCombo();
					}
					static int create_texid = 0;
					ImGui::SliderInt("Texture", &create_texid, 0, this->textures.size());
					if(ImGui::Button("Create"))
					{
						this->add_to_draw_list(create_meshid, {}, {static_cast<std::uint32_t>(create_texid), 0u});
					}
					ImGui::EndTabItem();
				}
				if(ImGui::BeginTabItem("Object Debugger"))
				{
					if(this->draw_list.size())
					{
						static int draw_id = 0;
						ImGui::SliderInt("Object ID", &draw_id, 0, this->draw_list.size() - 1);
						draw_id = std::min(static_cast<std::size_t>(draw_id), this->draw_list.size() - 1);

						// write the draw data.
						drawdata_storage_t& draw_data = tz::gl::get_device().get_renderer(this->rh).get_resource(this->drawdata_buf)->data_as<drawdata_storage_t>().front();
						drawdata_element_t& draw_elem = draw_data.draws[draw_id];

						if(ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
						{
							ImGui::Indent();
							ImGui::InputFloat4("#mx", draw_elem.model[0].data());
							ImGui::InputFloat4("#my", draw_elem.model[1].data());
							ImGui::InputFloat4("#mz", draw_elem.model[2].data());
							ImGui::InputFloat4("#m4", draw_elem.model[3].data());
							ImGui::Unindent();
						}

						if(ImGui::CollapsingHeader("Colour", ImGuiTreeNodeFlags_DefaultOpen))
						{
							ImGui::Indent();
							static int texel_id = 0;
							ImGui::SliderInt("Texture Element ID", &texel_id, 0, object_attached_texture_count - 1);
							ImGui::SliderInt("Texid", &reinterpret_cast<int&>(draw_elem.textures[texel_id].texid), 0, this->textures.size());
							ImGui::SliderFloat3("Tint Colour", draw_elem.textures[texel_id].tint.data().data(), 0.0f, 1.0f);
							ImGui::Unindent();
						}
					}
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Camera"))
		{
			bool changed = false;
			changed |= ImGui::SliderFloat3("Position", this->camera_intermediate_data.pos.data().data(), -75.0f, 75.0f);
			changed |= ImGui::SliderFloat3("Rotation", this->camera_intermediate_data.rot.data().data(), -3.14159f * 0.5f, 3.14159f * 0.5f);
			ImGui::Checkbox("Enable Mouse + Keyboard Movement", &this->allow_controls);
			ImGui::SliderFloat("Movement Speed", &this->camera_speed, 0.01f, 100.0f);
			ImGui::SliderFloat("Rotation Speed", &this->camera_rot_speed, 0.1f, 2.0f);
			if(changed)
			{
				this->write_camera_buffer();
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void mesh_renderer::update(float dt)
{
	if(!this->allow_controls)
	{
		return;
	}
	const auto& kstate = tz::window().get_keyboard_state();
	const auto& mstate = tz::window().get_mouse_state();
	static tz::vec2 old_mouse_pos;
	tz::vec2 mpos = mstate.mouse_position;
	bool cam_needs_update = false;
	if(tz::wsi::is_mouse_button_down(mstate, tz::wsi::mouse_button::left) && !tz::dbgui::claims_mouse())
	{
		tz::vec2 mouse_delta = mpos - old_mouse_pos;
		this->camera_intermediate_data.rot[1] -= mouse_delta[0] * this->camera_rot_speed * dt;
		this->camera_intermediate_data.rot[0] -= mouse_delta[1] * this->camera_rot_speed * dt;
		cam_needs_update = true;
	}
	old_mouse_pos = mpos;
	tz::vec3 move_dir = tz::vec3::zero();

	tz::mat4 temp_view = tz::view(this->camera_intermediate_data.pos, this->camera_intermediate_data.rot);
	if(tz::wsi::is_key_down(kstate, tz::wsi::key::w))
	{
		move_dir += (temp_view * tz::vec4{0.0f, 0.0f, -1.0f, 0.0f}).swizzle<0, 1, 2>();
	}
	if(tz::wsi::is_key_down(kstate, tz::wsi::key::s))
	{
		move_dir += (temp_view * tz::vec4{0.0f, 0.0f, 1.0f, 0.0f}).swizzle<0, 1, 2>();
	}
	if(tz::wsi::is_key_down(kstate, tz::wsi::key::a))
	{
		move_dir += (temp_view * tz::vec4{-1.0f, 0.0f, 0.0f, 0.0f}).swizzle<0, 1, 2>();
	}
	if(tz::wsi::is_key_down(kstate, tz::wsi::key::d))
	{
		move_dir += (temp_view * tz::vec4{1.0f, 0.0f, 0.0f, 0.0f}).swizzle<0, 1, 2>();
	}
	if(tz::wsi::is_key_down(kstate, tz::wsi::key::space))
	{
		move_dir += (temp_view * tz::vec4{0.0f, 1.0f, 0.0f, 0.0f}).swizzle<0, 1, 2>();
	}
	if(tz::wsi::is_key_down(kstate, tz::wsi::key::left_shift))
	{
		move_dir += (temp_view * tz::vec4{0.0f, -1.0f, 0.0f, 0.0f}).swizzle<0, 1, 2>();
	}
	if(move_dir != tz::vec3::zero())
	{
		move_dir = move_dir.normalised() * this->camera_speed * dt;
		this->camera_intermediate_data.pos += move_dir;
		cam_needs_update = true;
	}
	if(cam_needs_update)
	{
		this->write_camera_buffer();
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

void mesh_renderer::append_meshid_to_draw_buffer(meshid_t mesh, transform_t transform, std::vector<texid_t> tex)
{
	// write the draw data.
	drawdata_storage_t& draw_data = tz::gl::get_device().get_renderer(this->rh).get_resource(this->drawdata_buf)->data_as<drawdata_storage_t>().front();
	drawdata_element_t& elem = draw_data.draws[this->draw_list.size()];
	elem.model = tz::model(transform.pos, transform.rot, transform.scale);
	tz::assert(tex.size() == 2, "Textures supported: {colour, normal}. You have specified %zu, we want 2.", tex.size());
	for(std::size_t i = 0; i < tex.size(); i++)
	{
		elem.textures[i] = {.tint = {1.0f, 1.0f, 1.0f}, .texid = tex[i]};
	}
	// write out the mesh reference.
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
		voffset += entry.vtx_count;
		ioffset += entry.idx_count;
	}
	tz::error("Could not find mesh reference. Did you delete it?");
	return {};
}

void mesh_renderer::write_camera_buffer()
{
	camera_data_t& camdata = tz::gl::get_device().get_renderer(this->rh).get_resource(this->camera_buf)->data_as<camera_data_t>().front();
	camdata.v = tz::view(this->camera_intermediate_data.pos, this->camera_intermediate_data.rot);
	camdata.p = tz::perspective(3.14159f / 2.0f, 1920.0f / 1080.0f, 0.1f, 1000.0f);
}
