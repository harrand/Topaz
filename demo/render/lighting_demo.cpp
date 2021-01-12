#include "core/core.hpp"
#include "core/debug/print.hpp"
#include "geo/matrix_transform.hpp"
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
#include "dui/imgui_context.hpp"
#include "gl/resource_writer.hpp"
#include "gl/modules/bindless_sampler.hpp"
#include "render/asset.hpp"
#include "render/scene.hpp"
#include <unordered_map>

const char *vtx_shader_src = R"glsl(
	#version 460
	layout (location = 0) in vec3 aPos;
	layout (location = 1) in vec2 aTexcoord;
    layout (location = 2) in vec3 aNormal;
    struct MVP
    {
        mat4 m;
        mat4 v;
        mat4 p;
    };
	#ssbo matrices
	{
		MVP mvp[512];
	};
    out vec3 pos;
	out vec2 texcoord;
    out vec3 normal;
    out MVP cur_mvp;
	void main()
	{
        cur_mvp = mvp[gl_DrawID];
        mat4 curMVPMatrix = cur_mvp.p * cur_mvp.v * cur_mvp.m;
		gl_Position = curMVPMatrix * vec4(aPos.x, aPos.y, aPos.z, 1.0);
        pos = aPos;
		texcoord = aTexcoord;
        normal = aNormal;
	}
	)glsl";
const char *frg_shader_src = R"glsl(
	#version 460
	#extension GL_ARB_bindless_texture : require
	out vec4 FragColor;
    struct MVP
    {
        mat4 m;
        mat4 v;
        mat4 p;
    };
    in vec3 pos;
	in vec2 texcoord;
    in vec3 normal;
    in MVP cur_mvp;
	#ssbo texture_block
	{
		tz_bindless_sampler tex;
	};

    struct DirectionalLight
    {
        /// Direction should always be provided in cameraspace.
        vec3 direction;
        /// Colour should always follow RGB notation.
        vec3 colour;
        /// Arbitrary units, use your own range.
        float power;
    };

    struct PointLight
    {
        // alignment = 16
        //If the member is a three-component vector with components consuming N basic machine units, the base alignment is 4N
        // Position is in cameraspace.
        // 0
        vec3 position; // 12 (4 bytes padding to 16)
        vec3 colour; // 28 (4 bytes padding to 32)
        float power; // 36
    };

	PointLight parse_light(vec3 pos, vec3 col, float pow)
	{
		PointLight light;
		light.position = pos;
		light.colour = col;
		light.power = pow;
		return light;
	}

    vec3 diffuse_directional(DirectionalLight light, vec3 diffuse_colour, vec3 normal_cameraspace)
    {
        float cos_theta = clamp(dot(normal_cameraspace, light.direction), 0.0, 1.0);
        return diffuse_colour * light.colour * light.power * cos_theta;
    }

    vec3 diffuse(PointLight light, vec3 diffuse_colour, vec3 normal_cameraspace, vec3 position_cameraspace)
    {
		vec3 eye_direction_cameraspace = vec3(0, 0, 0) - position_cameraspace;
        float distance = length(light.position - position_cameraspace);
        DirectionalLight directional;
        directional.direction = light.position + eye_direction_cameraspace;
        directional.colour = light.colour;
        directional.power = light.power;
        return diffuse_directional(directional, diffuse_colour, normal_cameraspace) / pow(distance, 2);
    }

    #ssbo lighting_block
    {
		vec3 ambient_light;
        PointLight point_lights[];
    };

	void main()
	{
		vec4 col = texture(tex, texcoord);
        vec4 ambient_col = vec4(ambient_light, 1.0) * col;
        vec3 normal_cameraspace = (cur_mvp.v * cur_mvp.m * vec4(normal, 0.0)).xyz;
        vec3 position_cameraspace = (cur_mvp.v * cur_mvp.m * vec4(pos, 1.0)).xyz;
        vec3 forward_modelspace = vec3(0.0, 0.0, -1.0);
        vec3 forward_cameraspace = (cur_mvp.v * cur_mvp.m * vec4(forward_modelspace, 0.0)).xyz;
		vec4 output_colour = vec4(0.0, 0.0, 0.0, 1.0);
		for(int i = 0; i < point_lights.length(); i++)
		{
			PointLight cur_light = point_lights[i];
			output_colour += vec4(diffuse(cur_light, col.xyz, normal_cameraspace, position_cameraspace), 1.0);
		}

		FragColor = ambient_col + output_colour;
		// Debug purposes only
		//FragColor = vec4(cur_light.position, 1.0);
	}
	)glsl";

constexpr std::size_t max_lights = 32;

struct MVP
{
    tz::Mat4 m;
    tz::Mat4 v;
    tz::Mat4 p;
};

struct PointLight
{
public:
	PointLight(tz::Vec3 position, tz::Vec3 colour, float power): position(position), colour(colour), power(power){}

	tz::Vec3 position;
	float pad0;
	tz::Vec3 colour;
	float power;

};

class LightingDemoWindow : public tz::ext::imgui::ImGuiWindow
{
public:
    LightingDemoWindow(tz::Vec3* ambient_light, bool* rotate, std::array<PointLight*, max_lights> lights): tz::ext::imgui::ImGuiWindow("Topaz Lighting Demo"), ambient_light(ambient_light), rotate(rotate), lights(lights){}

    virtual void render() override
    {
        ImGui::Begin("Topaz Lighting Demo", &this->visible);
        ImGui::DragFloat3("Ambient Light", ambient_light->data(), 0.01f, 0.0f, 1.0f);
		ImGui::Checkbox("Rotation Enabled", this->rotate);
        for(std::size_t i = 0; i < max_lights; i++)
        {
            PointLight* light = this->lights[i];
            if(ImGui::TreeNode((std::string{"Light "} + std::to_string(i)).c_str()))
            {
                ImGui::DragFloat3("Position Worldspace", light->position.data(), 0.5f, -100.0f, 100.0f);
                ImGui::DragFloat3("Colour", light->colour.data(), 0.01f, 0.0f, 1.0f);
                ImGui::DragFloat("Power (W)", &light->power, 1.0f, 0.0f, 10000.0f);
                ImGui::TreePop();
            }
        }
        ImGui::End();
    }
private:
    tz::Vec3* ambient_light;
	bool* rotate;
    std::array<PointLight*, max_lights> lights;
};

class SplitTransformResourceWriter
{
public:
    SplitTransformResourceWriter(tz::mem::Block data): mat_writer(data){}
    bool write(tz::Vec3 pos, tz::Vec3 rot, tz::Vec3 scale, tz::Vec3 cam_pos, tz::Vec3 cam_rot, float fov, float aspect, float near, float far)
    {
        tz::Mat4 model = tz::geo::model(pos, rot, scale);
        tz::Mat4 view = tz::geo::view(cam_pos, cam_rot);
        tz::Mat4 proj = tz::geo::perspective(fov, aspect, near, far);
        MVP mvp{model, view, proj};
        return this->mat_writer.write(mvp);
    }

    void reset()
    {
        this->mat_writer.reset();
    }
private:
    tz::gl::BasicResourceWriter<MVP> mat_writer;
};

namespace tz::render
{
    template<>
    class ElementWriter<tz::render::SceneElement, SplitTransformResourceWriter>
    {
    public:
        static void write(SplitTransformResourceWriter& writer, const tz::render::SceneElement& ele)
        {
            writer.write(ele.transform.position, ele.transform.rotation, ele.transform.scale, ele.camera.position, ele.camera.rotation, ele.camera.fov, ele.camera.aspect_ratio, ele.camera.near, ele.camera.far);
        }
    };
}

int main()
{
    using MyScene = tz::render::Scene<tz::render::SceneElement, SplitTransformResourceWriter>;
	constexpr std::size_t max_elements = 512;
	// Minimalist Graphics Demo.
	tz::core::initialise("Topaz Lighting Demo");
	tz::core::get().enable_culling(false);
	{
		tz::gl::Manager m;
		tz::gl::Object& o = *m;

		// Track it in imgui.
		tz::ext::imgui::track_object(&o);

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
		topaz_assert(ssbo_module->size() == 3, "SSBO Module had unexpected number of buffers. Expected 3, got ", ssbo_module->size());
		std::size_t ssbo_id = ssbo_module->get_buffer_id(0);
		std::size_t tex_ssbo_id = ssbo_module->get_buffer_id(1);
        std::size_t light_ssbo_id = ssbo_module->get_buffer_id(2);

		tz::gl::SSBO* ssbo = o.get<tz::gl::BufferType::ShaderStorage>(ssbo_id);
		tz::gl::SSBO* tex_ssbo = o.get<tz::gl::BufferType::ShaderStorage>(tex_ssbo_id);
        tz::gl::SSBO* light_ssbo = o.get<tz::gl::BufferType::ShaderStorage>(light_ssbo_id);

		tz::gl::IndexedMesh triangle;
		triangle.vertices.push_back(tz::gl::Vertex{{{-0.5f, 0.5f, 0.0f}}, {{0.0f, 0.0f}}, {{0.0f, 0.0f, -1.0f}}, {{}}, {{}}});
		triangle.vertices.push_back(tz::gl::Vertex{{{0.5f, 0.5f, 0.0f}}, {{1.0f, 0.0f}}, {{0.0f, 0.0f, -1.0f}}, {{}}, {{}}});
		triangle.vertices.push_back(tz::gl::Vertex{{{0.0f, 1.5f, 0.0f}}, {{0.5f, 1.0f}}, {{0.0f, 0.0f, -1.0f}}, {{}}, {{}}});
		triangle.indices = {0, 1, 2};

		tz::gl::IndexedMesh square;
		square.vertices.push_back(tz::gl::Vertex{{{-0.5f, -0.5f, 0.0f}}, {{0.0f, 0.0f}}, {{}}, {{}}, {{}}});
		square.vertices.push_back(tz::gl::Vertex{{{0.5f, -0.5f, 0.0f}}, {{1.0f, 0.0f}}, {{}}, {{}}, {{}}});
		square.vertices.push_back(tz::gl::Vertex{{{0.5f, 0.5f, 0.0f}}, {{1.0f, 0.5f}}, {{}}, {{}}, {{}}});
		
		square.vertices.push_back(tz::gl::Vertex{{{-0.5f, -0.5f, 0.0f}}, {{0.0f, 0.0f}}, {{}}, {{}}, {{}}});
		square.vertices.push_back(tz::gl::Vertex{{{0.5f, 0.5f, 0.0f}}, {{1.0f, 0.5f}}, {{}}, {{}}, {{}}});
		square.vertices.push_back(tz::gl::Vertex{{{-0.5f, 0.5f, 0.0f}}, {{0.0f, 0.5f}}, {{}}, {{}}, {{}}});
		square.indices = {0, 1, 2, 3, 4, 5};

		tz::gl::IndexedMesh cube;
		cube.vertices.push_back(tz::gl::Vertex{{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}});
		cube.vertices.push_back(tz::gl::Vertex{{-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}});
		cube.vertices.push_back(tz::gl::Vertex{{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}});
		cube.vertices.push_back(tz::gl::Vertex{{1.0f, -1.0f, -1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}});

		cube.vertices.push_back(tz::gl::Vertex{{-1.0f, -1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}});
		cube.vertices.push_back(tz::gl::Vertex{{-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}});
		cube.vertices.push_back(tz::gl::Vertex{{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}});
		cube.vertices.push_back(tz::gl::Vertex{{1.0f, -1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}});

		cube.vertices.push_back(tz::gl::Vertex{{-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}});
		cube.vertices.push_back(tz::gl::Vertex{{-1.0f, -1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}});
		cube.vertices.push_back(tz::gl::Vertex{{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}});
		cube.vertices.push_back(tz::gl::Vertex{{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}});

		cube.vertices.push_back(tz::gl::Vertex{{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}});
		cube.vertices.push_back(tz::gl::Vertex{{-1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}});
		cube.vertices.push_back(tz::gl::Vertex{{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}});
		cube.vertices.push_back(tz::gl::Vertex{{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}});

		cube.vertices.push_back(tz::gl::Vertex{{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}});
		cube.vertices.push_back(tz::gl::Vertex{{-1.0f, -1.0f, 1.0f}, {1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}});
		cube.vertices.push_back(tz::gl::Vertex{{-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}});
		cube.vertices.push_back(tz::gl::Vertex{{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}});

		cube.vertices.push_back(tz::gl::Vertex{{1.0f, -1.0f, -1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}});
		cube.vertices.push_back(tz::gl::Vertex{{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}});
		cube.vertices.push_back(tz::gl::Vertex{{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}});
		cube.vertices.push_back(tz::gl::Vertex{{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}});

		cube.indices = {0, 1, 2,
						0, 2, 3,

						6, 5, 4,
						7, 6, 4,

						10, 9, 8,
						11, 10, 8,

						12, 13, 14,
						12, 14, 15,

						16, 17, 18,
						16, 18, 19,

						22, 21, 20,
						23, 22, 20
						};
		/**
		 * Vertex vertices[] =
	{

		Vertex(glm::vec3(1, -1, -1), glm::vec2(1, 1), glm::vec3(1, 0, 0)),
		Vertex(glm::vec3(1, -1, 1), glm::vec2(1, 0), glm::vec3(1, 0, 0)),
		Vertex(glm::vec3(1, 1, 1), glm::vec2(0, 0), glm::vec3(1, 0, 0)),
		Vertex(glm::vec3(1, 1, -1), glm::vec2(0, 1), glm::vec3(1, 0, 0)),
	};

	unsigned int indices[] = {0, 1, 2,
							  0, 2, 3,

							  6, 5, 4,
							  7, 6, 4,

							  10, 9, 8,
							  11, 10, 8,

							  12, 13, 14,
							  12, 14, 15,

							  16, 17, 18,
							  16, 18, 19,

							  22, 21, 20,
							  23, 22, 20
	                          };
		 * 
		 */

		tz::gl::IndexedMesh monkey_head = tz::gl::load_mesh("res/models/monkeyhead.obj");
		tz::Vec3 cam_pos{{0.0f, 0.0f, 5.0f}};
		// camera movement implementation
		auto add_pos = [&cam_pos](float x, float y, float z)
		{
			cam_pos[0] += x;
			cam_pos[1] += y;
			cam_pos[2] += z;
		};
		tz::core::IWindow& wnd = tz::core::get().window();
		wnd.register_this();
		wnd.emplace_custom_key_listener([&add_pos](tz::input::KeyPressEvent e)
		{
			switch(e.key)
			{
			case GLFW_KEY_W:
				add_pos(0.0f, 0.00f, -0.05f);
				tz::debug_printf("moving forward.\n");
			break;
			case GLFW_KEY_S:
				add_pos(0.0f, 0.00f, 0.05f);
				tz::debug_printf("moving backward.\n");
			break;
			case GLFW_KEY_A:
				add_pos(-0.05f, 0.0f, 0.0f);
				tz::debug_printf("moving left\n");
			break;
			case GLFW_KEY_D:
				add_pos(0.05f, 0.0f, 0.0f);
				tz::debug_printf("moving right\n");
			break;
			case GLFW_KEY_SPACE:
				add_pos(0.0f, 0.05f, 0.0f);
				tz::debug_printf("moving up\n");
			break;
			case GLFW_KEY_LEFT_SHIFT:
				add_pos(0.0f, -0.05f, 0.0f);
				tz::debug_printf("moving down\n");
			break;
			}
		});
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
		//tz::gl::Manager::Handle square_handle = m.add_mesh(square);
		//tz::gl::Manager::Handle monkeyhead_handle = m.add_mesh(monkey_head);
		tz::gl::Manager::Handle cube_handle = m.add_mesh(cube);

		// UBO stores mesh transform data (mvp)
		ssbo->terminal_resize(sizeof(tz::Mat4) * 3 * max_elements);
		// Scene uses UBO resource data.
		MyScene scene{ssbo->map()};

		float rotation_y = 0.0f;

		wnd.get_frame()->set_clear_color(0.3f, 0.15f, 0.0f);
		// Scene setup.
		// Meshes
		tz::render::AssetBuffer::Index triangle_mesh_idx = scene.add_mesh({&m, triangle_handle});
		//tz::render::AssetBuffer::Index monkey_mesh_idx = scene.add_mesh({&m, monkeyhead_handle});
		//tz::render::AssetBuffer::Index square_mesh_idx = scene.add_mesh({&m, square_handle});
		tz::render::AssetBuffer::Index cube_mesh_idx = scene.add_mesh({&m, cube_handle});
		// Textures
		tex_ssbo->resize(sizeof(tz::gl::BindlessTextureHandle));
		{
            tz::gl::BindlessTextureHandle metal_handle = metal.get_terminal_handle();
            tex_ssbo->send(&metal_handle);
		}
        // Lighting information
        constexpr std::size_t light_buf_size = sizeof(tz::Vec3) + sizeof(float) /*extra padding required*/ + (sizeof(PointLight) * max_lights) /*point lights*/;
        constexpr float default_ambient_pow = 0.2f;
		tz::Vec3* ambient_lighting;
        std::array<PointLight*, max_lights> lights;
        light_ssbo->terminal_resize(light_buf_size);
        {
		    tz::mem::Block blk = light_ssbo->map();
			ambient_lighting = new (blk.begin) tz::Vec3{default_ambient_pow, default_ambient_pow, default_ambient_pow};
			blk.begin = reinterpret_cast<char*>(blk.begin) + sizeof(tz::Vec3) + /* add extra float because glsl will align to that */sizeof(float);
            tz::mem::UniformPool<PointLight> light_pool{blk};
            for(std::size_t i = 0; i < max_lights; i++)
            {
                const PointLight default_light{tz::Vec3{}, tz::Vec3{0.5f, 0.0f, 0.0f}, 0.0f};
                light_pool.set(i, default_light);
                lights[i] = &light_pool[i];
            }
        }

		bool rotate_enabled = true;

        tz::ext::imgui::emplace_window<LightingDemoWindow>(ambient_lighting, &rotate_enabled, lights);

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
			tz::render::SceneElement ele{cube_mesh_idx};
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
        dev.add_resource_buffer(light_ssbo);
		dev.set_handle(m.get_indices());

		while(!wnd.is_close_requested())
		{
			if(rotate_enabled)
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
			tz::core::update();
			wnd.update();
		}
	}
	tz::core::terminate();
}