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
    struct MVP
    {
        mat4 m;
        mat4 v;
        mat4 p;
    };
	#ssbo matrices
	{
		MVP mvp_element[512];
	};
    out vec3 pos;
	out vec2 texcoord;
    out mat4 model;
    out mat4 view;
    out mat4 proj;
	void main()
	{
        MVP cur_mvp = mvp_element[gl_DrawID];
        // position, in camera space.
        pos = ((cur_mvp.v * cur_mvp.m) * vec4(aPos, 1.0)).xyz;
		texcoord = aTexcoord;
        model = cur_mvp.m;
        view = cur_mvp.v;
        proj = cur_mvp.p;
	}
	)glsl";
const char* tess_ctrl_src = R"glsl(
    #version 460
    layout (vertices = 3) out;

    // outputs from vertex shader.
    in vec3 pos[];
    in vec2 texcoord[];
    in mat4 model[];
    in mat4 view[];
    in mat4 proj[];

    // inputs to evaluation shader
    out vec3 pos_eval[];
    out vec2 texcoord_eval[];
    out mat4 model_eval[];
    out mat4 view_eval[];
    out mat4 proj_eval[];

    void main()
    {
        // Set the control points of the output patch.
        pos_eval[gl_InvocationID] = pos[gl_InvocationID];
        texcoord_eval[gl_InvocationID] = texcoord[gl_InvocationID];
        model_eval[gl_InvocationID] = model[gl_InvocationID];
        view_eval[gl_InvocationID] = view[gl_InvocationID];
        proj_eval[gl_InvocationID] = proj[gl_InvocationID];

        vec3 cam_pos_cs = vec3(0.0); // camera position, in camera space (duh!)
        float eye_to_vertex_dist0 = distance(cam_pos_cs, pos_eval[0]);
        float eye_to_vertex_dist1 = distance(cam_pos_cs, pos_eval[1]);
        float eye_to_vertex_dist2 = distance(cam_pos_cs, pos_eval[2]);

        // Calculate tessellation levels.
        // TODO: Change with respect to distance? The setup is all ready.
        gl_TessLevelOuter[0] = 8;
        gl_TessLevelOuter[1] = 8;
        gl_TessLevelOuter[2] = 8;
        gl_TessLevelInner[0] = gl_TessLevelOuter[2];
    }
)glsl";
const char* tess_eval_src = R"glsl(
    #version 460
    #extension GL_ARB_bindless_texture : require
    layout(triangles, equal_spacing, ccw) in;

    // inputs to evaluation shader
    in vec3 pos_eval[];
    in vec2 texcoord_eval[];
    in mat4 model_eval[];
    in mat4 view_eval[];
    in mat4 proj_eval[];

    out vec2 texcoord;

    vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)
    {
        return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
    }

    vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
    {
        return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
    }

    void main()
    {
        // Interpolate the attributes of the output vertex using the barycentric coordinates
        texcoord = interpolate2D(texcoord_eval[0], texcoord_eval[1], texcoord_eval[2]);
        vec3 pos = interpolate3D(pos_eval[0], pos_eval[1], pos_eval[2]);
        // remember this is in camera space. we only need multiply by p to get in clip space.
        gl_Position = proj_eval[0] * vec4(pos, 1.0);
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

class SeparateTransformResourceWriter : public tz::gl::TransformResourceWriter
{
public:
    SeparateTransformResourceWriter(tz::mem::Block data): tz::gl::TransformResourceWriter(data){}
    bool write(tz::Vec3 pos, tz::Vec3 rot, tz::Vec3 scale, tz::Vec3 cam_pos, tz::Vec3 cam_rot, float fov, float aspect, float near, float far)
    {
        tz::Mat4 model = tz::model(pos, rot, scale);
        tz::Mat4 view = tz::view(cam_pos, cam_rot);
        tz::Mat4 proj = tz::perspective(fov, aspect, near, far);
        // Writes the mvp components separately instead of at once.
        return this->mat_writer.write(model) && this->mat_writer.write(view) && this->mat_writer.write(proj);
    }
};

namespace tz::render
{
    template<>
    class ElementWriter<tz::render::SceneElement, SeparateTransformResourceWriter>
    {
    public:
        static void write([[maybe_unused]] SeparateTransformResourceWriter& writer, [[maybe_unused]] const tz::render::SceneElement& element)
        {
            writer.write(element.transform.position, element.transform.rotation, element.transform.scale, element.camera.position, element.camera.rotation, element.camera.fov, element.camera.aspect_ratio, element.camera.near, element.camera.far);
        }
    };
}

class MeshAdjustor : public tz::dui::DebugWindow
{
public:
	MeshAdjustor(tz::Vec3& offset, float& rotation_factor, float& x_spacing, float& y_spacing, tz::render::Scene<tz::render::SceneElement, SeparateTransformResourceWriter>& scene): DebugWindow("Mesh Adjustor"), offset(offset), rotation_factor(rotation_factor), x_spacing(x_spacing), y_spacing(y_spacing), scene(scene){}

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
	tz::render::Scene<tz::render::SceneElement, SeparateTransformResourceWriter>& scene;
	std::unordered_map<const char*, tz::render::AssetBuffer::Index> meshes;
};

int main()
{
	constexpr std::size_t max_elements = 1;
	constexpr std::size_t max_textures = 8;
	// Minimalist Graphics Demo.
	tz::initialise("Topaz Tessellation Demo");
	{
		tz::gl::Manager m;
		tz::gl::Object& o = *m;

		// Track it in imgui.
		tz::dui::track_object(&o);
        tz::get().enable_wireframe_mode(true);

		tz::gl::p::SSBOModule* ssbo_module = nullptr;
		std::string vtx_result;
		std::string frg_result;
		tz::gl::ShaderPreprocessor pre{vtx_shader_src};
		{
			std::size_t ssbo_module_id = pre.emplace_module<tz::gl::p::SSBOModule>(&o);
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

		// UBO stores mesh transform data (model + view + projection each)
		ssbo->terminal_resize(sizeof(tz::Mat4) * max_elements * 3);
		// Scene uses UBO resource data.
		tz::render::Scene<tz::render::SceneElement, SeparateTransformResourceWriter> scene{ssbo->map()};

		MeshAdjustor& mesh_adjustor = tz::dui::emplace_window<MeshAdjustor>(triangle_pos, rotation_factor, x_factor, y_factor, scene);

		tz::IWindow& wnd = tz::get().window();
		wnd.register_this();

		wnd.get_frame()->set_clear_color(0.3f, 0.15f, 0.0f);
		// Scene setup.
		// Meshes
		tz::render::AssetBuffer::Index triangle_mesh_idx = scene.add_mesh({m, triangle_handle});
		mesh_adjustor.register_mesh("triangle", triangle_mesh_idx);
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
        tz::gl::Shader* tcs = prg.emplace(tz::gl::ShaderType::TessellationControl, tess_ctrl_src);
        tz::gl::Shader* tes = prg.emplace(tz::gl::ShaderType::TessellationEvaluation, tess_eval_src);
		tz::gl::Shader* fs = prg.emplace(tz::gl::ShaderType::Fragment);
		fs->upload_source(frg_result);

		auto cpl_diag = cpl.compile(*vs);
		topaz_assert(cpl_diag.successful(), "Shader Vtx Compilation Fail: ", cpl_diag.get_info_log());
        cpl_diag = cpl.compile(*tcs);
		topaz_assert(cpl_diag.successful(), "Shader Tcs Compilation Fail: ", cpl_diag.get_info_log());
        cpl_diag = cpl.compile(*tes);
		topaz_assert(cpl_diag.successful(), "Shader Tcs Compilation Fail: ", cpl_diag.get_info_log());
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
        dev.set_is_patches(true);

		while(!wnd.is_close_requested())
		{

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