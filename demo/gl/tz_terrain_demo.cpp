#include "tz/core/tz.hpp"
#include "tz/core/window.hpp"
#include "tz/core/profiling/zone.hpp"
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
	});
	{
		struct BufferData
		{
			tz::Mat4 model = tz::model({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f});
			tz::Mat4 view = tz::view({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f});
			tz::Mat4 projection = tz::Mat4::identity();
			tz::Vec3 camera_position{0.0f, 0.0f, 0.0f};
		};

		struct FeedbackData
		{
			tz::Vec3 pos{0.0f, 0.0f, 0.0f};
		};


		tz::gl::BufferResource buf = tz::gl::BufferResource::from_one(BufferData{},
		{
			.access = tz::gl::ResourceAccess::DynamicFixed
		});
		tz::gl::BufferResource buf2 = tz::gl::BufferResource::from_one(FeedbackData{},
		{
			tz::gl::ResourceAccess::DynamicFixed
		});

		tz::gl::RendererInfo rinfo;
		tz::gl::ResourceHandle bufh = rinfo.add_resource(buf);
		tz::gl::ResourceHandle bufh2 = rinfo.add_resource(buf2);
		rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(tz_terrain_demo, vertex));
		rinfo.shader().set_shader(tz::gl::ShaderStage::TessellationControl, ImportedShaderSource(tz_terrain_demo, tesscon));
		rinfo.shader().set_shader(tz::gl::ShaderStage::TessellationEvaluation, ImportedShaderSource(tz_terrain_demo, tesseval));
		rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(tz_terrain_demo, fragment));
		rinfo.set_clear_colour({0.0f, 0.765f, 1.0f, 1.0f});

		tz::gl::RendererHandle rendererh = tz::gl::device().create_renderer(rinfo);
		tz::gl::Renderer& renderer = tz::gl::device().get_renderer(rendererh);
		tz::Vec3 cam_rot{0.0f, 0.0f, 0.0f};
		bool wireframe_mode = false;
		bool flight_enabled = false;
		bool game_menu_enabled = false;
		using namespace tz::literals;
		tz::Delay fixed_update{25_ms};

		tz::dbgui::game_menu().add_callback([&game_menu_enabled]()
		{
			ImGui::MenuItem("Control Panel", nullptr, &game_menu_enabled);
		});

		constexpr float multiplier = 3.5f;
		while(!tz::window().is_close_requested())
		{
			tz::window().begin_frame();
			renderer.render(4);

			tz::dbgui::run([&wireframe_mode, &flight_enabled, &game_menu_enabled]()
			{
				if(game_menu_enabled)
				{
					ImGui::Begin("Control Panel", &game_menu_enabled);
					ImGui::Checkbox("Flight Enabled", &flight_enabled);
					ImGui::Checkbox("Wireframe Enabled", &wireframe_mode);
					ImGui::End();
				}
			});

			// Every 25ms, we do a fixed-update.
			if(fixed_update.done())
			{
				fixed_update.reset();

				renderer.edit(tz::gl::RendererEditBuilder{}
					.render_state({.wireframe_mode = wireframe_mode})
					.build());
				// Retrieve the dynamic buffer resource data.
				BufferData& bufd = renderer.get_resource(bufh)->data_as<BufferData>().front();
				tz::Vec3& camera_position = bufd.camera_position;

				// Dragging the mouse influences the camera rotation.
				static tz::Vec2i mouse_position;
				auto mpi = static_cast<tz::Vec2i>(tz::window().get_mouse_position_state().get_mouse_position());
				if(tz::window().get_mouse_button_state().is_mouse_button_down(tz::MouseButton::Left) && !tz::dbgui::claims_mouse())
				{
					// Get mouse delta since last frame.
					tz::Vec2i mouse_delta = mpi - mouse_position;
					constexpr float rot_multiplier = 0.003f;
					cam_rot[1] -= mouse_delta[0] * rot_multiplier;
					cam_rot[0] -= mouse_delta[1] * rot_multiplier;
				}
				mouse_position = mpi;

				tz::Vec3 camera_position_height_only = camera_position;
				camera_position_height_only[0] = camera_position_height_only[2] = 0.0f;
				bufd.view = tz::view(camera_position_height_only, cam_rot);
				// Recalculate projection every fixed update. This is a massive waste of time but easily guarantees no distortion if the window is ever resized.
				const float aspect_ratio = static_cast<float>(tz::window().get_width()) / tz::window().get_height();
				bufd.projection = tz::perspective(1.6f, aspect_ratio, 0.1f, 1000.0f);
				// WASD move the camera position around. Space and LeftShift move camera directly up or down.

				tz::Vec4 cam_forward4 = bufd.view * tz::Vec4{0.0f, 0.0f, -1.0f, 0.0f};
				tz::Vec4 cam_right4 = bufd.view * tz::Vec4{-1.0f, 0.0f, 0.0f, 0.0f};
				tz::Vec3 cam_forward = cam_forward4.swizzle<0, 1, 2>();
				tz::Vec3 cam_right = cam_right4.swizzle<0, 1, 2>();
				if(tz::window().get_keyboard_state().is_key_down(tz::KeyCode::W))
				{
					camera_position += cam_forward * multiplier;
				}
				if(tz::window().get_keyboard_state().is_key_down(tz::KeyCode::S))
				{
					camera_position -= cam_forward * multiplier;
				}
				if(tz::window().get_keyboard_state().is_key_down(tz::KeyCode::A))
				{
					camera_position += cam_right * multiplier;
				}
				if(tz::window().get_keyboard_state().is_key_down(tz::KeyCode::D))
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
			tz::window().end_frame();
		}
	}
	tz::terminate();
}
