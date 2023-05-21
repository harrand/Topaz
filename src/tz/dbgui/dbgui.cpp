#include "tz/dbgui/dbgui.hpp"
#include "tz/wsi/keyboard.hpp"
#include "tz/wsi/mouse.hpp"
#include "tz/core/time.hpp"
#include "tz/core/debug.hpp"
#include "tz/core/job/job.hpp"
#include "tz/core/matrix_transform.hpp"
#include "tz/core/profile.hpp"
#include "tz/gl/output.hpp"
#include "tz/gl/renderer.hpp"
#include "tz/gl/resource.hpp"
#include "tz/gl/draw.hpp"
#include "tz/gl/imported_shaders.hpp"
#include "imgui.h"

#if TZ_VULKAN
#include "tz/gl/impl/vulkan/detail/tz_vulkan.hpp"
#elif TZ_OGL
#include "tz/gl/impl/opengl/detail/tz_opengl.hpp"
#endif

#include <memory>

#include ImportedShaderHeader(dbgui, vertex)
#include ImportedShaderHeader(dbgui, fragment)
#include ImportedShaderHeader(empty, vertex)
#include ImportedShaderHeader(empty, fragment)

namespace tz::dbgui
{
	struct TopazPlatformData
	{
		tz::game_info game_info;
		tz::wsi::keyboard_state kb_state;
		tz::wsi::mouse_state mouse_state;

		std::size_t frame_counter = 0;
		tz::duration last_update;
		float frame_period = 0.0f;
		tz::duration fps_update_duration;
	};

	struct TopazRenderData
	{
		tz::gl::renderer_handle renderer = tz::nullhand;
		tz::gl::renderer_handle final_renderer = tz::nullhand;
		tz::gl::resource_handle vertex_buffer = tz::nullhand;
		tz::gl::resource_handle index_buffer = tz::nullhand;
		tz::gl::resource_handle draw_buffer = tz::nullhand;
		tz::gl::resource_handle shader_data_buffer = tz::nullhand;
	};

	TopazPlatformData* global_platform_data = nullptr;
	TopazRenderData* global_render_data = nullptr;
	tz::game_info global_info;
	game_menu_callback_type game_menu_callback;
	game_bar_callback_type game_bar_callback;

	void imgui_impl_handle_inputs();

	bool imgui_impl_tz_init();
	void imgui_impl_tz_term();
	void imgui_impl_render();
	void imgui_impl_begin_commands();
	void imgui_impl_style_colours_purple();

	ImGuiKey tz_key_to_imgui(tz::wsi::key key_code);
	ImGuiMouseButton tz_btn_to_imgui(tz::wsi::mouse_button btn);

	ImTextureID handle_to_texid(tz::gl::resource_handle handle)
	{
		return reinterpret_cast<ImTextureID>(static_cast<std::uintptr_t>(static_cast<std::size_t>(static_cast<tz::hanval>(handle))));
	}

	void initialise([[maybe_unused]] init_info info)
	{
		TZ_PROFZONE("tz::dbgui::initialise", 0xFFAA00AA);
		#if TZ_DEBUG
			global_info = info.game_info;
			bool ret = imgui_impl_tz_init();
			tz::assert(ret, "Failed to initialise tz imgui backend.");
		#endif // TZ_DEBUG
	}

	void terminate()
	{
		TZ_PROFZONE("tz::dbgui::terminate", 0xFFAA00AA);
		#if TZ_DEBUG
			imgui_impl_tz_term();
		#endif // TZ_DEBUG
	}

	void begin_frame()
	{
		TZ_PROFZONE("tz::dbgui::begin_frame", 0xFFAA00AA);
		#if TZ_DEBUG
			ImGuiIO& io = ImGui::GetIO();
			auto dims = tz::window().get_dimensions();
			io.DisplaySize = ImVec2
			{
				(float)dims[0], (float)dims[1]
			};
			ImGui::NewFrame();
			imgui_impl_begin_commands();
		#endif // TZ_DEBUG
	}

	void end_frame()
	{
		TZ_PROFZONE("tz::dbgui::end_frame", 0xFFAA00AA);
		#if TZ_DEBUG
			ImGui::EndFrame();
			ImGui::Render();
			imgui_impl_render();

			imgui_impl_handle_inputs();

		#endif //TZ_DEBUG
	}

	game_menu_callback_type& game_menu()
	{
		return game_menu_callback;
	}

	game_bar_callback_type& game_bar()
	{
		return game_bar_callback;
	}

	bool claims_keyboard()
	{
		#if TZ_DEBUG
			return ImGui::GetIO().WantCaptureKeyboard;
		#endif
		return false;
	}

	bool claims_mouse()
	{
		#if TZ_DEBUG
			return ImGui::GetIO().WantCaptureMouse;
		#endif
		return false;
	}

	struct TopazShaderRenderData
	{
		tz::mat4 vp;
		std::uint32_t texture_id;
		std::uint32_t index_offset;
		std::uint32_t vertex_offset;
		float pad0;
	};

	enum class MouseWheelDirection
	{
		Up,
		Down,
		Same
	};

	struct InputDelta
	{
		std::vector<tz::wsi::key> newly_pressed = {};
		std::vector<tz::wsi::key> newly_released = {};
		std::vector<tz::wsi::mouse_button> newly_pressed_buttons = {};
		std::vector<tz::wsi::mouse_button> newly_released_buttons = {};
		bool mouse_position_changed = false;
		MouseWheelDirection mouse_wheel_dir = MouseWheelDirection::Same;
	};

	InputDelta imgui_impl_get_input_delta()
	{
		#if TZ_DEBUG
			auto before_keys = global_platform_data->kb_state.keys_down;
			auto after_keys = tz::window().get_keyboard_state().keys_down;
			std::vector<tz::wsi::key> newly_pressed;
			std::vector<tz::wsi::key> newly_released;
			for(tz::wsi::key k : before_keys)
			{
				if(k == tz::wsi::key::unknown)
					continue;
				// Anything that was down before but no longer is has been released.
				if(std::find(after_keys.begin(), after_keys.end(), k) == after_keys.end())
				{
					newly_released.push_back(k);
				}
			}
			for(tz::wsi::key k : after_keys)
			{
				if(k == tz::wsi::key::unknown)
					continue;
				// Anything that was not down before but is now down has been pressed.
				if(std::find(before_keys.begin(), before_keys.end(), k) == before_keys.end())
				{
					newly_pressed.push_back(k);
				}
			}

			// Mouse position and buttons
			bool mouse_moved = tz::window().get_mouse_state().mouse_position != global_platform_data->mouse_state.mouse_position;
			auto before_buttons = global_platform_data->mouse_state.button_state;
			auto after_buttons = tz::window().get_mouse_state().button_state;
			std::vector<tz::wsi::mouse_button> newly_pressed_buttons;
			std::vector<tz::wsi::mouse_button> newly_released_buttons;
			for(std::size_t i = 0; i < static_cast<int>(tz::wsi::mouse_button::_count); i++)
			{
				if(before_buttons[i] != tz::wsi::mouse_button_state::noclicked)
				{
					if(after_buttons[i] == tz::wsi::mouse_button_state::noclicked)
					{
						// This button was released.
						newly_released_buttons.push_back(static_cast<tz::wsi::mouse_button>(i));
					}
				}
				else
				{
					if(after_buttons[i] != tz::wsi::mouse_button_state::noclicked)
					{
						// This button was pressed.
						newly_pressed_buttons.push_back(static_cast<tz::wsi::mouse_button>(i));
					}
				}
			}
		
			int before_wheel_position = global_platform_data->mouse_state.wheel_position;
			int after_wheel_position = tz::window().get_mouse_state().wheel_position;
			MouseWheelDirection mdir = MouseWheelDirection::Same;
			if(after_wheel_position > before_wheel_position)
			{
				mdir = MouseWheelDirection::Up;
			}
			else if(before_wheel_position > after_wheel_position)
			{
				mdir = MouseWheelDirection::Down;
			}

			// Update global platform data.
			global_platform_data->kb_state = tz::window().get_keyboard_state();
			global_platform_data->mouse_state = tz::window().get_mouse_state();

			// Return results.
			return
			{
				.newly_pressed = std::move(newly_pressed),
				.newly_released = std::move(newly_released),
				.newly_pressed_buttons = std::move(newly_pressed_buttons),
				.newly_released_buttons = std::move(newly_released_buttons),
				.mouse_position_changed  = mouse_moved,
				.mouse_wheel_dir = mdir
			};
		#endif // TZ_DEBUG
		return {};
	}

	void imgui_impl_handle_inputs()
	{
		#if TZ_DEBUG
			ImGuiIO& io = ImGui::GetIO();
			InputDelta delta = imgui_impl_get_input_delta();
			// Pass to imgui.
			for(const auto& press : delta.newly_pressed)
			{
				io.AddKeyEvent(tz_key_to_imgui(press), true);
			}
			if(!delta.newly_pressed.empty())
			{
				// Note: This might be wrong.
				io.AddInputCharacter(tz::wsi::get_chars_typed(delta.newly_pressed.back(), global_platform_data->kb_state).back());
			}
			for(const auto& release : delta.newly_released)
			{
				io.AddKeyEvent(tz_key_to_imgui(release), false);
			}

			for(const auto& btn_press : delta.newly_pressed_buttons)
			{
				ImGuiMouseButton btn = tz_btn_to_imgui(btn_press);
				if(btn == ImGuiMouseButton_COUNT)
				{
					continue;
				}
				io.AddMouseButtonEvent(btn, true);
			}
			for(const auto& btn_release : delta.newly_released_buttons)
			{
				ImGuiMouseButton btn = tz_btn_to_imgui(btn_release);
				if(btn == ImGuiMouseButton_COUNT)
				{
					continue;
				}
				io.AddMouseButtonEvent(btn, false);
			}

			if(delta.mouse_position_changed)
			{
				const tz::vec2ui mpos = tz::window().get_mouse_state().mouse_position;
				io.AddMousePosEvent(mpos[0], mpos[1]);
			}
			if(delta.mouse_wheel_dir != MouseWheelDirection::Same)
			{
				if(delta.mouse_wheel_dir == MouseWheelDirection::Up)
				{
					io.AddMouseWheelEvent(0.0f, 1.0f);
				}
				else
				{
					io.AddMouseWheelEvent(0.0f, -1.0f);
				}
			}
		#endif // TZ_DEBUG
	}

	bool imgui_impl_tz_init()
	{
		ImGui::CreateContext();
		global_platform_data = new TopazPlatformData;
		global_render_data = new TopazRenderData;

		ImGuiIO& io = ImGui::GetIO();
		tz::assert(io.BackendPlatformUserData == NULL, "Already initialised imgui backend!");
		io.BackendPlatformName = "Topaz";
		io.BackendPlatformUserData = global_platform_data;
		io.BackendRendererName = "tz::gl";
		io.BackendRendererUserData = global_render_data;

		// Make renderer.
		// - Font texture image_resource.
		unsigned char* font_pixels;
		int font_width, font_height;
		io.Fonts->GetTexDataAsRGBA32(&font_pixels, &font_width, &font_height);
		std::span<unsigned char> font_data{font_pixels, static_cast<std::size_t>(font_width * font_height * 4)};

		// IB and VB have initial size of 1KiB.
		struct Kibibyte{char d[1024];};

		tz::gl::buffer_resource vertex_buffer = tz::gl::buffer_resource::from_one(Kibibyte{},
		{
			.access = tz::gl::resource_access::dynamic_variable
		});
		tz::gl::buffer_resource index_buffer = tz::gl::buffer_resource::from_one(Kibibyte{},
		{
			.access = tz::gl::resource_access::dynamic_variable
		});
		tz::gl::buffer_resource draw_buffer = tz::gl::buffer_resource::from_one(tz::gl::draw_indirect_command
		{
			.count = 0u,
			.first = 0
		},
		{
			.access = tz::gl::resource_access::dynamic_fixed,
			.flags = {tz::gl::resource_flag::draw_indirect_buffer}
		});
		tz::gl::buffer_resource shader_data_buffer = tz::gl::buffer_resource::from_one(TopazShaderRenderData{},
		{
			.access = tz::gl::resource_access::dynamic_fixed
		});

		tz::gl::image_resource font_image = tz::gl::image_resource::from_memory
		(
			font_data,
			{
				.format = tz::gl::image_format::RGBA32,
				.dimensions = tz::vec2ui{static_cast<unsigned int>(font_width), static_cast<unsigned int>(font_height)}
			}
		);

		tz::gl::window_output wout{tz::window()};

		tz::gl::renderer_info rinfo;
		global_render_data->vertex_buffer = rinfo.add_resource(vertex_buffer);
		global_render_data->shader_data_buffer = rinfo.add_resource(shader_data_buffer);
		global_render_data->index_buffer = rinfo.add_resource(index_buffer);
		global_render_data->draw_buffer = rinfo.add_resource(draw_buffer);
		rinfo.add_resource(font_image);
		rinfo.state().graphics.draw_buffer = global_render_data->draw_buffer;
		rinfo.shader().set_shader(tz::gl::shader_stage::vertex, ImportedShaderSource(dbgui, vertex));
		rinfo.shader().set_shader(tz::gl::shader_stage::fragment, ImportedShaderSource(dbgui, fragment));
		rinfo.set_options({tz::gl::renderer_option::no_clear_output, tz::gl::renderer_option::no_depth_testing, tz::gl::renderer_option::no_present, tz::gl::renderer_option::render_wait, tz::gl::renderer_option::_internal});
		rinfo.set_output(wout);
		rinfo.debug_name("Dbgui Renderer");
		
		global_render_data->renderer = tz::gl::get_device().create_renderer(rinfo);

		tz::gl::renderer_info empty;
		empty.shader().set_shader(tz::gl::shader_stage::vertex, ImportedShaderSource(empty, vertex));
		empty.shader().set_shader(tz::gl::shader_stage::fragment, ImportedShaderSource(empty, fragment));
		empty.set_options({tz::gl::renderer_option::no_clear_output, tz::gl::renderer_option::no_depth_testing, tz::gl::renderer_option::_internal_final_dbgui_renderer, tz::gl::renderer_option::_internal});
		empty.debug_name("Dbgui Present");
		global_render_data->final_renderer = tz::gl::get_device().create_renderer(empty);

		io.Fonts->SetTexID(0);

		imgui_impl_style_colours_purple();

		return true;
	}

	void imgui_impl_tz_term()
	{
		delete global_platform_data;
		delete global_render_data;
		global_platform_data = nullptr;
		global_render_data = nullptr;

		ImGui::DestroyContext();
	}

	void imgui_impl_render()
	{
		TZ_PROFZONE("Dbgui Render", 0xFFAA00AA);

		ImDrawData* draw = ImGui::GetDrawData();
		tz::assert(draw != nullptr, "Null imgui draw data!");
		tz::assert(draw->Valid, "Invalid draw data!");

		tz::assert(global_render_data->renderer != tz::nullhand, "Null imgui renderer when trying to render!");
		// We have a font texture already.
		tz::gl::renderer& renderer = tz::gl::get_device().get_renderer(global_render_data->renderer);
		// We have no idea how big our vertex/index buffers need to be. Let's copy over the data now.
		const auto req_idx_size = static_cast<std::size_t>(draw->TotalIdxCount) * sizeof(ImDrawIdx);
		const auto req_vtx_size = static_cast<std::size_t>(draw->TotalVtxCount) * sizeof(ImDrawVert);
		{
			tz::gl::RendererEditBuilder edit;
			if(renderer.get_resource(global_render_data->index_buffer)->data().size_bytes() < req_idx_size)
			{
				edit.buffer_resize
				({
					.buffer_handle = global_render_data->index_buffer,
					.size = req_idx_size
				});
			}
			if(renderer.get_resource(global_render_data->vertex_buffer)->data().size_bytes() < req_vtx_size)
			{
				edit.buffer_resize
				({
					.buffer_handle = global_render_data->vertex_buffer,
					.size = req_vtx_size
				});
			}
			static_assert(sizeof(ImDrawIdx) == sizeof(unsigned int), "Topaz indices must be c++ unsigned ints under-the-hood. ImDrawIdx does not match its size.");
			{
				TZ_PROFZONE("Dbgui Render - IB/VB Resize", 0xFFAA00AA);
				renderer.edit(edit.build());
			}
		}
		auto indices = renderer.get_resource(global_render_data->index_buffer)->data();
		auto vertices = renderer.get_resource(global_render_data->vertex_buffer)->data();
		// Copy over the vertex and index data.
		std::memset(indices.data(), 0, indices.size_bytes());
		std::memset(vertices.data(), 0, vertices.size_bytes());
		// Copy over all the vertex and index data.
		std::size_t index_cursor = 0;
		std::size_t vertex_cursor = 0;
		for(std::size_t n = 0; std::cmp_less(n, draw->CmdListsCount); n++)
		{
			TZ_PROFZONE("Dbgui Render - Command List Processing", 0xFFAA00AA);
			const ImDrawList* cmd = draw->CmdLists[n];
			std::memcpy(indices.data() + index_cursor, cmd->IdxBuffer.Data, cmd->IdxBuffer.Size * sizeof(ImDrawIdx));
			std::memcpy(vertices.data() + vertex_cursor, cmd->VtxBuffer.Data, cmd->VtxBuffer.Size * sizeof(ImDrawVert));
			// Set shader data (view-projection and texture-id)
			TopazShaderRenderData& shader_data = renderer.get_resource(global_render_data->shader_data_buffer)->data_as<TopazShaderRenderData>().front();
			const ImGuiIO& io = ImGui::GetIO();
			shader_data.vp = tz::orthographic(
				0,
				io.DisplaySize.x,
				0,
				io.DisplaySize.y,
				-0.1f,
				0.1f
			) * tz::view(tz::vec3{io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f, 0.0f}, {});
			for(const ImDrawCmd& draw_cmd : cmd->CmdBuffer)
			{
				TZ_PROFZONE("Dbgui Render - Single Command Processing", 0xFFAA00AA);
				shader_data.texture_id = static_cast<std::size_t>(reinterpret_cast<std::uintptr_t>(draw_cmd.TextureId));
				shader_data.index_offset = draw_cmd.IdxOffset;
				shader_data.vertex_offset = draw_cmd.VtxOffset;

				ImVec2 min = {draw_cmd.ClipRect.x, draw_cmd.ClipRect.y};
				ImVec2 max = {draw_cmd.ClipRect.z - draw_cmd.ClipRect.x, draw_cmd.ClipRect.w - draw_cmd.ClipRect.y};
				auto offset = static_cast<tz::vec2ui>(tz::vec2{min.x, min.y} - tz::vec2{draw->DisplayPos.x, draw->DisplayPos.y});
				auto extent = static_cast<tz::vec2ui>(tz::vec2{max.x, max.y});
#if TZ_OGL
				const tz::gl::ioutput* output = renderer.get_output();
				extent[1] = io.DisplaySize.y - output->scissor.extent[1] - output->scissor.offset[1];
#endif
				tz::gl::RendererEditBuilder edit;
				edit.set_scissor({.offset = offset, .extent = extent});

				// TODO: Do the edit and apply the scissor rectangle. Uncomment the below line to actually do this, potentially incorrect results without scissor rectangles, however avoiding this renderer edit will yield a ~4x speed improvement.
				//renderer.edit(edit.build());
				renderer.get_resource(global_render_data->draw_buffer)->data_as<tz::gl::draw_indexed_indirect_command>().front() =
				{
					.count = draw_cmd.ElemCount,
					.instance_count = 1u,
					.first_index = 0u,
					.base_vertex = 0,
					.base_instance = 0u
				};
				if(draw_cmd.UserCallback == nullptr)
				{
					renderer.render();
				}
				else
				{
					draw_cmd.UserCallback(cmd, &draw_cmd);
				}
			}
		}
		{
			TZ_PROFZONE("Dbgui Render - Final Pass", 0xFFAA00AA);
			tz::gl::get_device().get_renderer(global_render_data->final_renderer).render();
		}
	}

	struct ImGuiTabTZ
	{
		bool show_top_bar = true;
		bool show_window_info = false;
		bool show_device_info = false;
	};

	ImGuiTabTZ tab_tz;

	void draw_tz_info()
	{
		#if TZ_VULKAN
			ImGui::Text("Vulkan");					
			ImGui::SameLine();
			tz::version ver = tz::gl::vk2::vulkan_version;
			ImGui::Text("%u.%u |", ver.major, ver.minor);
		#elif TZ_OGL
			ImGui::Text("OpenGL");	
			ImGui::SameLine();
			tz::version ver = tz::gl::ogl2::ogl_version;
			ImGui::Text("%u.%u Core Profile |", ver.major, ver.minor);
		#else
			ImGui::Text("Unknown");	
		#endif
		ImGui::SameLine();
		#if TZ_DEBUG
			ImGui::Text("Debug");
		#elif TZ_PROFILE
			ImGui::Text("Profile");
		#else
			ImGui::Text("Release");
		#endif
		ImGui::SameLine();
		#if defined(__GNUC__)
			ImGui::Text("GCC");
		#elif defined(__clang__)
			ImGui::Text("Clang");
		#elif defined(_MSC_VER)
			ImGui::Text("MSVC");
		#else
			ImGui::Text("Unknown");
		#endif

		ImGui::SameLine();
		#if defined(_WIN64)
			ImGui::Text("Windows |");
		#elif defined(__unix__)
			ImGui::Text("Unix |");	
		#else
			ImGui::Text("Unknown OS |");
		#endif

		ImGui::SameLine();
		ImGui::Text("Frame %zu", global_platform_data->frame_counter);
		ImGui::SameLine();
		ImGui::Text("%.2ffps |", 1000.0f / global_platform_data->frame_period);
		ImGui::SameLine();
		ImGui::Text("%u jobs", tz::job_system().jobs_started_this_frame());

		//if(ImGui::CollapsingHeader("Graphics Backend"))
		//{
		//	#if TZ_VULKAN
		//		const tz::gl::vk2::VulkanInstance& vinst = tz::gl::vk2::get();
		//		ImGui::Text("Validation Layers Enabled: %s", vinst.validation_layers_enabled() ? "true" : "false");
		//		ImGui::Text("Vulkan Instance Extensions:");
		//		for(tz::gl::vk2::InstanceExtension iext : vinst.get_extensions())
		//		{
		//			ImGui::Text("- %s (%s)", tz::gl::vk2::util::instance_extension_tz_names[static_cast<int>(iext)], tz::gl::vk2::util::instance_extension_names[static_cast<int>(iext)]);
		//		}
		//		const tz::gl::vk2::LogicalDevice& ldev = tz::gl::get_device().vk_get_logical_device();
		//		ImGui::Text("Vulkan device Extensions:");
		//		for(tz::gl::vk2::DeviceExtension dext : ldev.get_extensions())
		//		{
		//			ImGui::Text("- %s (%s)", tz::gl::vk2::util::device_extension_tz_names[static_cast<int>(dext)], tz::gl::vk2::util::device_extension_names[static_cast<int>(dext)]);
		//		}

		//		ImGui::Spacing();
		//		for(const tz::gl::vk2::PhysicalDevice& pdev : tz::gl::vk2::get_all_devices())
		//		{
		//			tz::gl::vk2::PhysicalDeviceInfo pinfo = pdev.get_info();
		//			if(ImGui::CollapsingHeader(pinfo.name.c_str()))
		//			{
		//				ImGui::Text("Used: %d", pdev == ldev.get_hardware());
		//				ImGui::Text("Vendor:");
		//				ImGui::SameLine();
		//				switch(pinfo.vendor)
		//				{
		//					case tz::gl::vk2::PhysicalDeviceVendor::Nvidia:
		//						ImGui::Text("Nvidia");
		//					break;
		//					case tz::gl::vk2::PhysicalDeviceVendor::AMD:
		//						ImGui::Text("AMD");
		//					break;
		//					case tz::gl::vk2::PhysicalDeviceVendor::Intel:
		//						ImGui::Text("Intel");
		//					break;
		//					case tz::gl::vk2::PhysicalDeviceVendor::Other:
		//						ImGui::Text("Unknown");
		//					break;
		//				}
		//				ImGui::Text("Type:");
		//				ImGui::SameLine();
		//				switch(pinfo.type)
		//				{
		//					case tz::gl::vk2::PhysicalDeviceType::IntegratedGPU:
		//						ImGui::Text("Integrated GPU");
		//					break;
		//					case tz::gl::vk2::PhysicalDeviceType::DiscreteGPU:
		//						ImGui::Text("Discrete GPU");
		//					break;
		//					case tz::gl::vk2::PhysicalDeviceType::VirtualGPU:
		//						ImGui::Text("Virtual GPU");
		//					break;
		//					case tz::gl::vk2::PhysicalDeviceType::CPU:
		//						ImGui::Text("CPU");
		//					break;
		//					default:
		//						ImGui::Text("Unknown");
		//					break;
		//				}
		//			}
		//		}
		//	#elif TZ_OGL
		//		ImGui::Text("Supports Bindless Textures: %d", tz::gl::ogl2::supports_bindless_textures());
		//	#endif
		//}

	//	if(ImGui::Button("Purple Style"))
	//	{
	//		imgui_impl_style_colours_purple();
	//	}
	}

	void draw_tz_window_info()
	{
		if(ImGui::Begin("Window", &tab_tz.show_window_info))
		{
			auto& wnd = tz::window();
			if(!(wnd.get_flags() & tz::wsi::window_flag::noresize))
			{
				auto xy = static_cast<tz::vec2i>(wnd.get_dimensions());
				if(ImGui::DragInt2("Dimensions", xy.data().data(), 3, 1, 4192, "%u"))
				{
					tz::assert(xy[0] > 0 && xy[1] > 0);
					wnd.set_dimensions(static_cast<tz::vec2ui>(xy));
				}
			}
			else
			{
				tz::vec2ui dims = wnd.get_dimensions();
				ImGui::Text("Dimensions = {%u, %u} - Fixed-size window", dims[0], dims[1]);
			}
			//ImGui::Text("Minimised = %s", wnd.is_minimised() ? "true" : "false");
			//ImGui::Text("Maximised = %s", wnd.is_maximised() ? "true" : "false");
			//ImGui::Text("Focused = %s", wnd.is_focused() ? "true" : "false");
			ImGui::End();
		}

	}

	void draw_tz_device_info()
	{
		if(ImGui::Begin("Device", &tab_tz.show_device_info))
		{
			tz::gl::get_device().dbgui();
			ImGui::End();
		}
	}

	void imgui_impl_begin_commands()
	{
		if(ImGui::BeginMainMenuBar())
		{
			if(ImGui::BeginMenu("Engine"))
			{
				ImGui::MenuItem("Top Bar", nullptr, &tab_tz.show_top_bar);
				ImGui::MenuItem("Window", nullptr, &tab_tz.show_window_info);
				ImGui::MenuItem("Device", nullptr, &tab_tz.show_device_info);
				if(ImGui::MenuItem("Debug Breakpoint"))
				{
					tz::error("Manual debug breakpoint occurred.");
				}
				ImGui::EndMenu();
			}
			if(!game_menu_callback.empty() && ImGui::BeginMenu(global_info.name))
			{
				game_menu_callback();
				ImGui::EndMenu();
			}	
			ImGui::EndMainMenuBar();

			if(tab_tz.show_top_bar)
			{
				ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
				ImGui::Begin("Runtime", nullptr,
					//ImGuiWindowFlags_NoBackground |
					ImGuiWindowFlags_NoDecoration |
					ImGuiWindowFlags_NoMove |
					ImGuiWindowFlags_NoInputs);
				ImGui::SetWindowPos(ImVec2(0, ImGui::GetFrameHeight()), true);
				auto sz = ImGui::GetWindowSize();
				sz.x = ImGui::GetIO().DisplaySize.x;
				sz.y *= 0.5f;
				ImGui::SetWindowSize(sz);
				draw_tz_info();
				ImGui::PopStyleColor();
				ImGui::End();
			}

			if(!game_bar_callback.empty())
			{
				ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
				ImGui::Begin("Game Runtime", nullptr,
					//ImGuiWindowFlags_NoBackground |
					ImGuiWindowFlags_NoDecoration |
					ImGuiWindowFlags_NoMove |
					ImGuiWindowFlags_NoInputs);
				ImGui::SetWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - 32), true);
				auto sz = ImGui::GetWindowSize();
				sz.x = ImGui::GetIO().DisplaySize.x;
				sz.y *= 0.5f;
				ImGui::SetWindowSize(sz);
				game_bar_callback();
				ImGui::PopStyleColor();
				ImGui::End();
			}


			if(tab_tz.show_window_info)
			{
				draw_tz_window_info();
			}
			if(tab_tz.show_device_info)
			{
				draw_tz_device_info();
			}
		}

		global_platform_data->frame_counter++;
		auto now = tz::system_time();
		if(now.millis<std::uint64_t>() > global_platform_data->fps_update_duration.millis<std::uint64_t>() + 500)
		{
			global_platform_data->frame_period = (now - global_platform_data->last_update).millis<float>();
			global_platform_data->fps_update_duration = now;
		}
		global_platform_data->last_update = now;
	}

	void imgui_impl_style_colours_purple()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImGui::StyleColorsDark(&style);

		ImVec4* colours = style.Colors;
		for(int i = ImGuiCol_Text; i < ImGuiCol_COUNT; i++)
		{
			std::swap(colours[i].y, colours[i].x);
			if(colours[i].x < 0.98f)
				colours[i].x *= 0.5f;
			colours[i].w = 1.0f;
		}
		colours[ImGuiCol_CheckMark].x *= 2.0f;
		colours[ImGuiCol_CheckMark].y *= 2.0f;
		colours[ImGuiCol_CheckMark].z *= 2.0f;
	}

	ImGuiMouseButton tz_btn_to_imgui(tz::wsi::mouse_button button)
	{
		switch(button)
		{
			case tz::wsi::mouse_button::left:
				return ImGuiMouseButton_Left;
			break;
			case tz::wsi::mouse_button::right:
				return ImGuiMouseButton_Right;
			break;
			case tz::wsi::mouse_button::middle:
				return ImGuiMouseButton_Middle;
			break;
			default:
				return ImGuiMouseButton_COUNT;
			break;
		}
	}

	ImGuiKey tz_key_to_imgui(tz::wsi::key key_code)
	{
		switch(key_code)
		{
			default:
			break;
			case tz::wsi::key::esc:
				return ImGuiKey_Escape;
			break;
			case tz::wsi::key::f1:
				return ImGuiKey_F1;
			break;
			case tz::wsi::key::f2:
				return ImGuiKey_F2;
			break;
			case tz::wsi::key::f3:
				return ImGuiKey_F3;
			break;
			case tz::wsi::key::f4:
				return ImGuiKey_F4;
			break;
			case tz::wsi::key::f5:
				return ImGuiKey_F5;
			break;
			case tz::wsi::key::f6:
				return ImGuiKey_F6;
			break;
			case tz::wsi::key::f7:
				return ImGuiKey_F7;
			break;
			case tz::wsi::key::f8:
				return ImGuiKey_F8;
			break;
			case tz::wsi::key::f9:
				return ImGuiKey_F9;
			break;
			case tz::wsi::key::f10:
				return ImGuiKey_F10;
			break;
			case tz::wsi::key::f11:
				return ImGuiKey_F11;
			break;
			case tz::wsi::key::f12:
				return ImGuiKey_F12;
			break;
			case tz::wsi::key::del:
				return ImGuiKey_Delete;
			break;
			case tz::wsi::key::page_up:
				return ImGuiKey_PageUp;
			break;
			case tz::wsi::key::page_down:
				return ImGuiKey_PageDown;
			break;
			case tz::wsi::key::print_screen:
				return ImGuiKey_PrintScreen;
			break;
			case tz::wsi::key::scroll_lock:
				return ImGuiKey_ScrollLock;
			break;
			case tz::wsi::key::pause:
				return ImGuiKey_Pause;
			break;
			case tz::wsi::key::insert:
				return ImGuiKey_Insert;
			break;
			case tz::wsi::key::one:
				return ImGuiKey_1;
			break;
			case tz::wsi::key::two:
				return ImGuiKey_2;
			break;
			case tz::wsi::key::three:
				return ImGuiKey_3;
			break;
			case tz::wsi::key::four:
				return ImGuiKey_4;
			break;
			case tz::wsi::key::five:
				return ImGuiKey_5;
			break;
			case tz::wsi::key::six:
				return ImGuiKey_6;
			break;
			case tz::wsi::key::seven:
				return ImGuiKey_7;
			break;
			case tz::wsi::key::eight:
				return ImGuiKey_8;
			break;
			case tz::wsi::key::nine:
				return ImGuiKey_9;
			break;
			case tz::wsi::key::zero:
				return ImGuiKey_0;
			break;
			case tz::wsi::key::minus:
				return ImGuiKey_Minus;
			break;
			case tz::wsi::key::equals:
				return ImGuiKey_Equal;
			break;
			case tz::wsi::key::backspace:
				return ImGuiKey_Backspace;
			break;
			case tz::wsi::key::tab:
				return ImGuiKey_Tab;
			break;
			case tz::wsi::key::q:
				return ImGuiKey_Q;
			break;
			case tz::wsi::key::w:
				return ImGuiKey_W;
			break;
			case tz::wsi::key::e:
				return ImGuiKey_E;
			break;
			case tz::wsi::key::r:
				return ImGuiKey_R;
			break;
			case tz::wsi::key::t:
				return ImGuiKey_T;
			break;
			case tz::wsi::key::y:
				return ImGuiKey_Y;
			break;
			case tz::wsi::key::u:
				return ImGuiKey_U;
			break;
			case tz::wsi::key::i:
				return ImGuiKey_I;
			break;
			case tz::wsi::key::o:
				return ImGuiKey_O;
			break;
			case tz::wsi::key::p:
				return ImGuiKey_P;
			break;
			case tz::wsi::key::left_bracket:
				return ImGuiKey_LeftBracket;
			break;
			case tz::wsi::key::right_bracket:
				return ImGuiKey_RightBracket;
			break;
			case tz::wsi::key::enter:
				return ImGuiKey_Enter;
			break;
			case tz::wsi::key::caps_lock:
				return ImGuiKey_CapsLock;
			break;
			case tz::wsi::key::a:
				return ImGuiKey_A;
			break;
			case tz::wsi::key::s:
				return ImGuiKey_S;
			break;
			case tz::wsi::key::d:
				return ImGuiKey_D;
			break;
			case tz::wsi::key::f:
				return ImGuiKey_F;
			break;
			case tz::wsi::key::g:
				return ImGuiKey_G;
			break;
			case tz::wsi::key::h:
				return ImGuiKey_H;
			break;
			case tz::wsi::key::j:
				return ImGuiKey_J;
			break;
			case tz::wsi::key::k:
				return ImGuiKey_K;
			break;
			case tz::wsi::key::l:
				return ImGuiKey_L;
			break;
			case tz::wsi::key::semi_colon:
				return ImGuiKey_Semicolon;
			break;
			case tz::wsi::key::apostrophe:
				return ImGuiKey_Apostrophe;
			break;
			case tz::wsi::key::hash:
				// none?
			break;
			case tz::wsi::key::left_shift:
				return ImGuiKey_LeftShift;
			break;
			case tz::wsi::key::backslash:
				return ImGuiKey_Backslash;
			break;
			case tz::wsi::key::z:
				return ImGuiKey_Z;
			break;
			case tz::wsi::key::x:
				return ImGuiKey_X;
			break;
			case tz::wsi::key::c:
				return ImGuiKey_C;
			break;
			case tz::wsi::key::v:
				return ImGuiKey_V;
			break;
			case tz::wsi::key::b:
				return ImGuiKey_B;
			break;
			case tz::wsi::key::n:
				return ImGuiKey_N;
			break;
			case tz::wsi::key::m:
				return ImGuiKey_M;
			break;
			case tz::wsi::key::comma:
				return ImGuiKey_Comma;
			break;
			case tz::wsi::key::period:
				return ImGuiKey_Period;
			break;
			case tz::wsi::key::forward_slash:
				return ImGuiKey_Slash;
			break;
			case tz::wsi::key::right_shift:
				return ImGuiKey_RightShift;
			break;
			case tz::wsi::key::left_ctrl:
				return ImGuiKey_LeftCtrl;
			break;
			case tz::wsi::key::win_key:
				// none?
			break;
			case tz::wsi::key::alt:
				return ImGuiKey_LeftAlt;
			break;
			case tz::wsi::key::space:
				return ImGuiKey_Space;
			break;
			case tz::wsi::key::alt_gr:
				return ImGuiKey_RightAlt;
			break;
			case tz::wsi::key::right_ctrl:
				return ImGuiKey_RightCtrl;
			break;
		}
		return ImGuiKey_None;
	}
}
