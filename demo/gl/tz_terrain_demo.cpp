#include "tz/core/tz.hpp"
#include "tz/wsi/keyboard.hpp"
#include "tz/wsi/mouse.hpp"
#include "hdk/profile.hpp"
#include "tz/core/matrix_transform.hpp"
#include "tz/core/time.hpp"
#include "tz/dbgui/dbgui.hpp"
#include "tz/gl/device.hpp"
#include "tz/gl/renderer.hpp"
#include "tz/gl/resource.hpp"
#include "tz/gl/imported_shaders.hpp"

#include ImportedShaderHeader(tz_terrain_demo, vertex)
#include ImportedShaderHeader(tz_terrain_demo, tesscon)
#include ImportedShaderHeader(tz_terrain_demo, tesseval)
#include ImportedShaderHeader(tz_terrain_demo, fragment)

int main()
{
	tz::initialise
	({
		.name = "tz_terrain_demo",
		.flags = {tz::application_flag::TransparentWindow}
	});
	{
		struct BufferData
		{
			tz::mat4 model = tz::model({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f});
			tz::mat4 view = tz::view({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f});
			tz::mat4 projection = tz::mat4::identity();
			hdk::vec3 camera_position{0.0f, 0.0f, 0.0f};
		};

		struct FeedbackData
		{
			hdk::vec3 pos{0.0f, 0.0f, 0.0f};
		};


		tz::gl::BufferResource buf = tz::gl::BufferResource::from_one(BufferData{},
		{
			.access = tz::gl::resource_access::dynamic_fixed
		});
		tz::gl::BufferResource buf2 = tz::gl::BufferResource::from_one(FeedbackData{},
		{
			tz::gl::resource_access::dynamic_fixed
		});

		tz::gl::renderer_info rinfo;
		tz::gl::resource_handle bufh = rinfo.add_resource(buf);
		tz::gl::resource_handle bufh2 = rinfo.add_resource(buf2);
		rinfo.shader().set_shader(tz::gl::shader_stage::vertex, ImportedShaderSource(tz_terrain_demo, vertex));
		rinfo.shader().set_shader(tz::gl::shader_stage::tessellation_control, ImportedShaderSource(tz_terrain_demo, tesscon));
		rinfo.shader().set_shader(tz::gl::shader_stage::tessellation_evaluation, ImportedShaderSource(tz_terrain_demo, tesseval));
		rinfo.shader().set_shader(tz::gl::shader_stage::fragment, ImportedShaderSource(tz_terrain_demo, fragment));
		//rinfo.state().graphics.clear_colour = {0.0f, 0.765f, 1.0f, 1.0f};

		tz::gl::renderer_handle rendererh = tz::gl::get_device().create_renderer(rinfo);
		tz::gl::renderer& renderer = tz::gl::get_device().get_renderer(rendererh);
		hdk::vec3 cam_rot{0.0f, 0.0f, 0.0f};
		bool flight_enabled = false;
		bool game_menu_enabled = false;
		using namespace tz::literals;
		tz::delay fixed_update{25_ms};

		tz::dbgui::game_menu().add_callback([&game_menu_enabled]()
		{
			ImGui::MenuItem("Control Panel", nullptr, &game_menu_enabled);
		});

		constexpr float multiplier = 3.5f;
		while(!tz::window().is_close_requested())
		{
			tz::begin_frame();
			renderer.render(4);

			tz::dbgui::run([&flight_enabled, &game_menu_enabled]()
			{
				if(game_menu_enabled)
				{
					ImGui::Begin("Control Panel", &game_menu_enabled);
					ImGui::Checkbox("Flight Enabled", &flight_enabled);
					ImGui::End();
				}
			});

			// Every 25ms, we do a fixed-update.
			if(fixed_update.done())
			{
				fixed_update.reset();
				// Retrieve the dynamic buffer resource data.
				BufferData& bufd = renderer.get_resource(bufh)->data_as<BufferData>().front();
				hdk::vec3& camera_position = bufd.camera_position;

				// Dragging the mouse influences the camera rotation.
				static hdk::vec2ui mouse_position;
				const auto& ms = tz::window().get_mouse_state();
				hdk::vec2ui mpi = ms.mouse_position;
				if(tz::wsi::is_mouse_button_down(ms, tz::wsi::mouse_button::left) && !tz::dbgui::claims_mouse())
				{
					// Get mouse delta since last frame.
					hdk::vec2i mouse_delta = mpi - mouse_position;
					constexpr float rot_multiplier = 0.003f;
					cam_rot[1] -= mouse_delta[0] * rot_multiplier;
					cam_rot[0] -= mouse_delta[1] * rot_multiplier;
				}
				mouse_position = mpi;

				hdk::vec3 camera_position_height_only = camera_position;
				camera_position_height_only[0] = camera_position_height_only[2] = 0.0f;
				bufd.view = tz::view(camera_position_height_only, cam_rot);
				// Recalculate projection every fixed update. This is a massive waste of time but easily guarantees no distortion if the window is ever resized.
				auto fdims = static_cast<hdk::vec2>(tz::window().get_dimensions());
				const float aspect_ratio = fdims[0] / fdims[1];
				bufd.projection = tz::perspective(1.6f, aspect_ratio, 0.1f, 1000.0f);
				// WASD move the camera position around. Space and LeftShift move camera directly up or down.

				hdk::vec4 cam_forward4 = bufd.view * hdk::vec4{0.0f, 0.0f, -1.0f, 0.0f};
				hdk::vec4 cam_right4 = bufd.view * hdk::vec4{-1.0f, 0.0f, 0.0f, 0.0f};
				hdk::vec3 cam_forward = cam_forward4.swizzle<0, 1, 2>();
				hdk::vec3 cam_right = cam_right4.swizzle<0, 1, 2>();
				const auto& kb = tz::window().get_keyboard_state();
				if(tz::wsi::is_key_down(kb, tz::wsi::key::w))
				{
					camera_position += cam_forward * multiplier;
				}
				if(tz::wsi::is_key_down(kb, tz::wsi::key::s))
				{
					camera_position -= cam_forward * multiplier;
				}
				if(tz::wsi::is_key_down(kb, tz::wsi::key::a))
				{
					camera_position += cam_right * multiplier;
				}
				if(tz::wsi::is_key_down(kb, tz::wsi::key::d))
				{
					camera_position -= cam_right * multiplier;
				}

				if(!flight_enabled)
				{
					float output_vertex_height = renderer.get_resource(bufh2)->data_as<FeedbackData>().front().pos[1];
					float dist_to_terrain = output_vertex_height - camera_position[1];
					camera_position[1] += std::clamp(dist_to_terrain * 0.3f, -5.0f, 5.0f);
				}
			}
			tz::end_frame();
		}
	}
	tz::terminate();
}
