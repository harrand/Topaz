#include "core/tz.hpp"
#include "core/window.hpp"
#include "core/profiling/zone.hpp"
#include "core/matrix_transform.hpp"
#include "core/time.hpp"
#include "gl/device.hpp"
#include "gl/renderer.hpp"
#include "gl/resource.hpp"
#include "gl/imported_shaders.hpp"

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
			tz::Mat4 view = tz::Mat4::identity();
			tz::Mat4 projection = tz::Mat4::identity();
		};

		tz::gl::Device dev;

		tz::gl::BufferResource buf = tz::gl::BufferResource::from_one(BufferData{}, tz::gl::ResourceAccess::DynamicFixed);

		tz::gl::RendererInfo rinfo;
		tz::gl::ResourceHandle bufh = rinfo.add_resource(buf);
		rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(tz_terrain_demo, vertex));
		rinfo.shader().set_shader(tz::gl::ShaderStage::TessellationControl, ImportedShaderSource(tz_terrain_demo, tesscon));
		rinfo.shader().set_shader(tz::gl::ShaderStage::TessellationEvaluation, ImportedShaderSource(tz_terrain_demo, tesseval));
		rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(tz_terrain_demo, fragment));

		tz::gl::Renderer renderer = dev.create_renderer(rinfo);
		tz::Vec3 camera_position{0.0f, 2.0f, 1.0f};
		tz::Vec3 cam_rot{0.0f, 0.0f, 0.0f};
		bool wireframe_mode = false;
		using namespace tz::literals;
		tz::Delay fixed_update{25_ms};

		constexpr float multiplier = 0.6f;
		while(!tz::window().is_close_requested())
		{
			TZ_FRAME_BEGIN;
			tz::window().update();
			renderer.render(2);
			TZ_FRAME_END;

			// Every 25ms, we do a fixed-update.
			if(fixed_update.done())
			{
				fixed_update.reset();

				// If Q is pressed, toggle wireframe mode via renderer edit.
				if(tz::window().get_keyboard_state().is_key_down(tz::KeyCode::Q))
				{
					renderer.edit
					({
						.render_state_edit = tz::gl::RendererStateEditRequest
						{
							.wireframe_mode = wireframe_mode
						}
					});
					wireframe_mode = !wireframe_mode;
				}
				// Retrieve the dynamic buffer resource data.
				BufferData& bufd = renderer.get_resource(bufh)->data_as<BufferData>().front();

				// Dragging the mouse influences the camera rotation.
				static tz::Vec2i mouse_position;
				auto mpi = static_cast<tz::Vec2i>(tz::window().get_mouse_position_state().get_mouse_position());
				if(tz::window().get_mouse_button_state().is_mouse_button_down(tz::MouseButton::Left))
				{
					// Get mouse delta since last frame.
					tz::Vec2i mouse_delta = mpi - mouse_position;
					constexpr float rot_multiplier = 0.003f;
					cam_rot[1] -= mouse_delta[0] * rot_multiplier;
					cam_rot[0] -= mouse_delta[1] * rot_multiplier;
				}
				mouse_position = mpi;

				bufd.view = tz::view(camera_position, cam_rot);
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
				if(tz::window().get_keyboard_state().is_key_down(tz::KeyCode::Space))
				{
					camera_position[1] += multiplier;
				}
				if(tz::window().get_keyboard_state().is_key_down(tz::KeyCode::LeftShift))
				{
					camera_position[1] -= multiplier;
				}
				if(tz::window().get_keyboard_state().is_key_down(tz::KeyCode::A))
				{
					camera_position += cam_right * multiplier;
				}
				if(tz::window().get_keyboard_state().is_key_down(tz::KeyCode::D))
				{
					camera_position -= cam_right * multiplier;
				}

			}
		}
	}
	tz::terminate();
}
