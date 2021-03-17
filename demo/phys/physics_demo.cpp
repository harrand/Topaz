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
#include "memory/polymorphic_variant.hpp"

#include "phys/world.hpp"
#include "phys/colliders/sphere.hpp"
#include "phys/resolvers/position.hpp"
#include "phys/resolvers/impulse.hpp"
#include <unordered_map>
#include <chrono>

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

class TrackedPhysicsWorld : public tz::phys::World
{
public:
    void add_body(tz::phys::Body& body)
    {
        this->body_tracker.push_back(&body);
        tz::phys::World::add_body(body);
    }

	void update(float delta_millis)
	{
		if(!time_frozen)
		{
			tz::phys::World::update(delta_millis);
		}
	}

    std::vector<tz::phys::Body*> body_tracker;
	bool time_frozen = false;
};

class PhysicsWorldAdjustor : public tz::dui::DebugWindow
{
public:
    PhysicsWorldAdjustor(TrackedPhysicsWorld& world): tz::dui::DebugWindow("Physics World Adjustor"), world(world){}

    virtual void render() override
    {
        ImGui::Begin("Physics World Adjustor", &this->visible);
		ImGui::Checkbox("Freeze Time", &this->world.time_frozen);
		std::size_t count = 0;
        for(tz::phys::Body* body : this->world.body_tracker)
        {
			ImGui::PushID(count++);
			ImGui::Spacing();
			ImGui::Text("Dynamics");
            ImGui::DragFloat3("Force: ", body->force.data());
            ImGui::DragFloat3("Position: ", body->transform.position.data());
            ImGui::DragFloat3("Velocity: ", body->velocity.data());
            ImGui::DragFloat("Mass: ", &body->mass, 0.25f, 0.0f, 1000.0f);
			ImGui::Text("Collision Response");
			ImGui::DragFloat("Restitution (Elasticity)", &body->restitution, 0.05f, 0.0f, 1.0f);
			ImGui::DragFloat("Static Friction", &body->static_friction, 0.1f, 0.0f, 20.0f);
			ImGui::DragFloat("Dynamic Friction", &body->dynamic_friction, 0.1f, 0.0f, 20.0f);
			ImGui::PopID();
        }
        ImGui::End();
    }
private:
    TrackedPhysicsWorld& world;
};

unsigned long long get_current_time()
{
    using namespace std::chrono;
    milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    return ms.count();  
}

int main()
{
	constexpr std::size_t max_elements = 2;
	constexpr std::size_t max_textures = 8;
	tz::initialise("Topaz Physics Demo");
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
		triangle.vertices.push_back(tz::gl::Vertex{{{-0.5f, -0.5f, 0.0f}}, {{0.0f, 0.0f}}, {{}}, {{}}, {{}}});
		triangle.vertices.push_back(tz::gl::Vertex{{{0.5f, -0.5f, 0.0f}}, {{1.0f, 0.0f}}, {{}}, {{}}, {{}}});
		triangle.vertices.push_back(tz::gl::Vertex{{{0.0f, 0.5f, 0.0f}}, {{0.5f, 1.0f}}, {{}}, {{}}, {{}}});
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

		tz::gl::Manager::Handle triangle_handle = m.add_mesh(triangle);
		tz::gl::Manager::Handle square_handle = m.add_mesh(square);

		// UBO stores mesh transform data (mvp)
		ssbo->terminal_resize(sizeof(tz::Mat4) * max_elements);
		// Scene uses UBO resource data.
		tz::render::Scene<tz::render::SceneElement> scene{ssbo->map()};

		tz::IWindow& wnd = tz::get().window();
		wnd.register_this();

		wnd.get_frame()->set_clear_color(0.3f, 0.15f, 0.0f);
		// Scene setup.
		// Meshes
		tz::render::AssetBuffer::Index triangle_mesh_idx = scene.add_mesh({m, triangle_handle});
		tz::render::AssetBuffer::Index square_mesh_idx = scene.add_mesh({m, square_handle});
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

		// By default, all elements are triangles.
		for(std::size_t i = 0; i < max_elements; i++)
		{
			tz::render::SceneElement ele{triangle_mesh_idx};
			ele.transform.position = tz::Vec3{1.0f, 1.0f, 1.0f};
			ele.transform.rotation = tz::Vec3{0.0f, 0.0f, 0.0f};
			ele.camera.fov = 1.57f;
			ele.camera.aspect_ratio = 1920.0f/1080.0f;
			ele.camera.near = 0.1f;
			ele.camera.far = 1000.0f;
			ele.camera.position = cam_pos;
			scene.add(ele);
		}

		// Element 1 is an exception, and will be a square instead.
		scene.get(1).mesh = square_mesh_idx;

		tz::render::Device dev{wnd.get_frame(), &prg, &o};
		dev.add_resource_buffer(ssbo);
		dev.add_resource_buffer(tex_ssbo);
		dev.set_handle(m.get_indices());

        // Physics stuff.
		tz::phys::SphereCollider common_collider{tz::Vec3{0.0f, 0.0f, 0.0f}, 0.5f};
        tz::phys::Body body
        {
            scene.get(0).transform,
            tz::Vec3{0.0f, 0.0f, 0.0f},
            tz::Vec3{0.0f, 0.0f, 0.0f},
            1.0f,
			0.5f,
			10.0f,
			2.0f,
			common_collider
        };
		body.transform.position = {0.0f, 2.0f, 0.0f};
		tz::phys::Body floor_body
		{
			scene.get(1).transform,
			tz::Vec3{0.0f, 0.0f, 0.0f},
			tz::Vec3{0.0f, 0.0f, 0.0f},
			100000.0f
		};
		floor_body.transform.position = {0.0f, -2.0f, 0.0f};
		floor_body.collider = common_collider;

		tz::phys::PositionResolver pos_res;
		tz::phys::ImpulseResolver inv_res;

        TrackedPhysicsWorld world;
        world.add_body(body);
		world.add_body(floor_body);
		//world.add_resolver(pos_res);
		world.add_resolver(inv_res);
        world.register_uniform_force({0.0f, -0.25f, 0.0f});

        tz::dui::emplace_window<PhysicsWorldAdjustor>(world).visible = true;

		while(!wnd.is_close_requested())
		{
            static unsigned long long old_time = get_current_time();
            unsigned long long new_time = get_current_time();
            world.update(static_cast<float>(new_time - old_time));
            old_time = new_time;
			dev.clear();
			o.bind();
			scene.configure(dev);
			dev.render();
			tz::update();
			wnd.update();
		}
	}
	tz::terminate();
}