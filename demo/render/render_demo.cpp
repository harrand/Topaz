#include "core/tz.hpp"
#include "core/debug/assert.hpp"
#include "core/debug/print.hpp"
#include "core/matrix_transform.hpp"
#include "gl/tz_stb_image/image_reader.hpp"
#include "gl/shader.hpp"
#include "gl/shader_compiler.hpp"
#include "gl/manager.hpp"
#include "gl/mesh_loader.hpp"
#include "gl/buffer.hpp"
#include "gl/frame.hpp"
#include "gl/modules/ssbo.hpp"
#include "gl/texture.hpp"
#include "render/device.hpp"
#include "GLFW/glfw3.h"
#include "dui/window.hpp"
#include "gl/resource_writer.hpp"
#include "gl/modules/bindless_sampler.hpp"
#include "render/asset.hpp"
#include "render/scene.hpp"
#include <unordered_map>

const char *vtx_shader_src = R"glsl(
	#version 460
	layout (location = 0) in vec3 aPos;
	layout (location = 1) in vec2 aTexcoord;
	#ssbo matrices
	{
		mat4 mvp[512];
	};
	out vec2 texcoord;
	void main()
	{
		gl_Position = mvp[gl_DrawID] * vec4(aPos.x, aPos.y, aPos.z, 1.0);
		texcoord = aTexcoord;
	}
	)glsl";
const char *frg_shader_src = R"glsl(
	#version 460
	#extension GL_ARB_bindless_texture : require
	out vec4 FragColor;
	in vec2 texcoord;
	#ssbo texture_block
	{
		tz_bindless_sampler textures[8];
	};
	void main()
	{
		FragColor = texture(textures[1], texcoord);
	}
	)glsl";

class MeshAdjustor : public tz::dui::DebugWindow
{
public:
	MeshAdjustor(tz::Vec3& offset, float& rotation_factor, float& x_spacing, float& y_spacing, tz::render::Scene<tz::render::SceneElement>& scene): DebugWindow("Mesh Adjustor"), offset(offset), rotation_factor(rotation_factor), x_spacing(x_spacing), y_spacing(y_spacing), scene(scene){}

	void register_mesh(const char* name, tz::render::AssetBuffer::Index index)
	{
		this->meshes.emplace(name, index);
	}

	virtual void render() override
	{
		ImGui::Begin("Mesh Adjustor", &this->visible);
		ImGui::Text("%s", "All Objects");
		ImGui::SliderFloat("Mesh Offset X", &offset[0], -100.0f, 100.0f);
		ImGui::SliderFloat("Mesh Offset Y", &offset[1], -100.0f, 100.0f);
		ImGui::SliderFloat("Mesh Offset Z", &offset[2], -100.0f, 100.0f);
		ImGui::SliderFloat("Horizontal Spacing Factor", &x_spacing, 0.0f, 5.0f);
		ImGui::SliderFloat("Vertical Spacing Factor", &y_spacing, 0.0f, 5.0f);
		ImGui::InputFloat("Rotation Factor", &rotation_factor);
		
		if(ImGui::TreeNode("Scene Elements"))
		{
			std::size_t i = 0;
			for(tz::render::SceneElement& ele : this->scene)
			{
				ImGui::SetNextItemOpen(true, ImGuiCond_Once);
				auto ptr = [](std::size_t i){return reinterpret_cast<void*>(static_cast<std::intptr_t>(i));};
				if(ImGui::TreeNode(ptr(i), "Element %zu", i))
				{
					ImGui::Text("Mesh:");
					for(const auto&[name, mesh] : this->meshes)
					{
						ImGui::SameLine();
						ImGui::RadioButton(name, reinterpret_cast<int*>(&ele.mesh), mesh);
					}
					ImGui::TreePop();
				}
				i++;
			}
			ImGui::TreePop();
		}
		ImGui::End();
	}
private:
	tz::Vec3& offset;
	float& rotation_factor;
	float& x_spacing;
	float& y_spacing;
	tz::render::Scene<tz::render::SceneElement>& scene;
	std::unordered_map<const char*, tz::render::AssetBuffer::Index> meshes;
};

int main()
{
	constexpr std::size_t max_elements = 512;
	constexpr std::size_t max_textures = 8;
	// Minimalist Graphics Demo.
	tz::initialise("Topaz Render Demo");
	{
		tz::get().render_settings().set_culling(tz::RenderSettings::CullTarget::Nothing);
		tz::gl::Manager m;
		tz::gl::Object& o = *m;

		// Track it in imgui.
		tz::dui::track_object(&o);

		tz::gl::p::SSBOModule* ssbo_module = nullptr;
		std::string vtx_result;
		std::string frg_result;
		tz::gl::ShaderPreprocessor pre{vtx_shader_src};
		{
			std::size_t ssbo_module_id = pre.emplace_module<tz::gl::p::SSBOModule>(o);
			pre.emplace_module<tz::gl::p::BindlessSamplerModule>();
			pre.preprocess();
			vtx_result = pre.result();
			pre.set_source(frg_shader_src);
			pre.preprocess();
			frg_result = pre.result();
			ssbo_module = static_cast<tz::gl::p::SSBOModule*>(pre[ssbo_module_id]);
		}
		topaz_assert(ssbo_module->size() == 2, "UBO Module had unexpected number of buffers. Expected 2, got ", ssbo_module->size());
		std::size_t ssbo_id = ssbo_module->get_buffer_id(0);
		std::size_t tex_ssbo_id = ssbo_module->get_buffer_id(1);

		tz::gl::SSBO* ssbo = o.get<tz::gl::BufferType::ShaderStorage>(ssbo_id);
		tz::gl::SSBO* tex_ssbo = o.get<tz::gl::BufferType::ShaderStorage>(tex_ssbo_id);

		tz::gl::IndexedMesh triangle;
		triangle.vertices.push_back(tz::gl::Vertex{{{-0.5f, 0.5f, 0.0f}}, {{0.0f, 0.0f}}, {{}}, {{}}, {{}}});
		triangle.vertices.push_back(tz::gl::Vertex{{{0.5f, 0.5f, 0.0f}}, {{1.0f, 0.0f}}, {{}}, {{}}, {{}}});
		triangle.vertices.push_back(tz::gl::Vertex{{{0.0f, 1.5f, 0.0f}}, {{0.5f, 1.0f}}, {{}}, {{}}, {{}}});
		triangle.indices = {0, 1, 2};

		tz::gl::IndexedMesh square;
		square.vertices.push_back(tz::gl::Vertex{{{-0.5f, -0.5f, 0.0f}}, {{0.0f, 0.0f}}, {{}}, {{}}, {{}}});
		square.vertices.push_back(tz::gl::Vertex{{{0.5f, -0.5f, 0.0f}}, {{1.0f, 0.0f}}, {{}}, {{}}, {{}}});
		square.vertices.push_back(tz::gl::Vertex{{{0.5f, 0.5f, 0.0f}}, {{1.0f, 0.5f}}, {{}}, {{}}, {{}}});
		
		square.vertices.push_back(tz::gl::Vertex{{{-0.5f, -0.5f, 0.0f}}, {{0.0f, 0.0f}}, {{}}, {{}}, {{}}});
		square.vertices.push_back(tz::gl::Vertex{{{0.5f, 0.5f, 0.0f}}, {{1.0f, 0.5f}}, {{}}, {{}}, {{}}});
		square.vertices.push_back(tz::gl::Vertex{{{-0.5f, 0.5f, 0.0f}}, {{0.0f, 0.5f}}, {{}}, {{}}, {{}}});
		square.indices = {0, 1, 2, 3, 4, 5};

		tz::gl::IndexedMesh monkey_head = tz::gl::load_mesh("res/models/monkeyhead.obj");
		const tz::Vec3 cam_pos{{0.0f, 0.0f, 5.0f}};
		tz::debug_printf("monkey head data size = %zu bytes, indices size = %zu bytes", monkey_head.data_size_bytes(), monkey_head.indices_size_bytes());

		tz::gl::Image<tz::gl::PixelRGB8> rgba_checkerboard = tz::ext::stb::read_image<tz::gl::PixelRGB8>("res/textures/bricks.jpg");
		tz::gl::Texture checkerboard;
		checkerboard.set_parameters(tz::gl::default_texture_params);
		checkerboard.set_data(rgba_checkerboard);
		checkerboard.make_terminal();
		
		tz::gl::Image<tz::gl::PixelRGB8> metal_img = tz::ext::stb::read_image<tz::gl::PixelRGB8>("res/textures/metal.jpg");
		tz::gl::Texture metal;
		metal.set_parameters(tz::gl::default_texture_params);
		metal.set_data(metal_img);
		metal.make_terminal();

		tz::Vec3 triangle_pos{{0.0f, 0.0f, 0.0f}};
		float rotation_factor = 0.02f;
		float x_factor = 2.5f;
		float y_factor = 2.5f;

		tz::gl::Manager::Handle triangle_handle = m.add_mesh(triangle);
		tz::gl::Manager::Handle square_handle = m.add_mesh(square);
		tz::gl::Manager::Handle monkeyhead_handle = m.add_mesh(monkey_head);

		// UBO stores mesh transform data (mvp)
		ssbo->terminal_resize(sizeof(tz::Mat4) * max_elements);
		// Scene uses UBO resource data.
		tz::render::Scene<tz::render::SceneElement> scene{ssbo->map()};

		MeshAdjustor& mesh_adjustor = tz::dui::emplace_window<MeshAdjustor>(triangle_pos, rotation_factor, x_factor, y_factor, scene);

		float rotation_y = 0.0f;

		tz::IWindow& wnd = tz::get().window();
		wnd.register_this();

		wnd.get_frame()->set_clear_color(0.3f, 0.15f, 0.0f);
		// Scene setup.
		// Meshes
		tz::render::AssetBuffer::Index triangle_mesh_idx = scene.add_mesh({m, triangle_handle});
		tz::render::AssetBuffer::Index monkey_mesh_idx = scene.add_mesh({m, monkeyhead_handle});
		tz::render::AssetBuffer::Index square_mesh_idx = scene.add_mesh({m, square_handle});
		mesh_adjustor.register_mesh("triangle", triangle_mesh_idx);
		mesh_adjustor.register_mesh("monkey head", monkey_mesh_idx);
		mesh_adjustor.register_mesh("square", square_mesh_idx);
		// Textures
		tex_ssbo->resize(sizeof(tz::gl::BindlessTextureHandle) * max_textures);
		{
			tz::mem::UniformPool<tz::gl::BindlessTextureHandle> tex_pool = tex_ssbo->map_uniform<tz::gl::BindlessTextureHandle>();
			tz::gl::BindlessTextureHandle checkerboard_handle = checkerboard.get_terminal_handle();
			tz::gl::BindlessTextureHandle metal_handle = metal.get_terminal_handle();
			tex_pool.set(0, metal_handle);
			tex_pool.set(1, checkerboard_handle);
			tex_ssbo->unmap();
		}

		tz::gl::ShaderCompiler cpl;
		tz::gl::ShaderProgram prg;
		tz::gl::Shader* vs = prg.emplace(tz::gl::ShaderType::Vertex);
		vs->upload_source(vtx_result);
		tz::gl::Shader* fs = prg.emplace(tz::gl::ShaderType::Fragment);
		fs->upload_source(frg_result);

		auto cpl_diag = cpl.compile(*vs);
		topaz_assert(cpl_diag.successful(), "Shader Vtx Compilation Fail: ", cpl_diag.get_info_log());
		cpl_diag = cpl.compile(*fs);
		topaz_assert(cpl_diag.successful(), "Shader Frg Compilation Fail: ", cpl_diag.get_info_log());

		auto lnk_diag = cpl.link(prg);
		topaz_assert(lnk_diag.successful(), "Shader Linkage Fail: ", lnk_diag.get_info_log());

		for(std::size_t i = 0; i < max_elements; i++)
		{
			tz::render::SceneElement ele{triangle_mesh_idx};
			ele.transform.position = tz::Vec3{1.0f, 1.0f, 1.0f};
			ele.transform.rotation = tz::Vec3{0.0f, 0.0f, 0.0f};
			ele.camera.fov = 1.57f;
			ele.camera.aspect_ratio = 1920.0f/1080.0f;
			ele.camera.near = 0.1f;
			ele.camera.far = 1000.0f;
			scene.add(ele);
		}

		tz::render::Device dev{wnd.get_frame(), &prg, &o};
		dev.add_resource_buffer(ssbo);
		dev.add_resource_buffer(tex_ssbo);
		dev.set_handle(m.get_indices());

		while(!wnd.is_close_requested())
		{
			rotation_y += rotation_factor;

			dev.clear();
			o.bind();
			for(std::size_t i = 0; i < max_elements; i++)
			{
				tz::Vec3 cur_pos = triangle_pos;
				// Three meshes in a horizontal line.
				cur_pos[0] += (i % static_cast<std::size_t>(std::sqrt(max_elements))) * x_factor;
				cur_pos[1] += std::floor(i / std::sqrt(max_elements)) * y_factor;

				tz::render::SceneElement& cur_ele = scene.get(i);
				cur_ele.transform.position = cur_pos;
				cur_ele.transform.rotation[1] = rotation_y;
				cur_ele.camera.position = cam_pos;
			}
			scene.configure(dev);
			dev.render();
			tz::update();
			wnd.update();
		}
	}
	tz::terminate();
}