#include "tz/dbgui/dbgui.hpp"
#include "tz/core/time.hpp"
#include "hdk/debug.hpp"
#include "hdk/job/job.hpp"
#include "tz/core/window.hpp"
#include "tz/core/matrix_transform.hpp"
#include "hdk/profile.hpp"
#include "tz/gl/output.hpp"
#include "tz/gl/renderer.hpp"
#include "tz/gl/resource.hpp"
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
		tz::GameInfo game_info;
		tz::KeyboardState kb_state;
		tz::MousePositionState mouse_pos_state;
		tz::MouseButtonState mouse_button_state;

		std::size_t frame_counter = 0;
		tz::Duration last_update;
		float frame_period = 0.0f;
		tz::Delay fps_update{tz::literals::operator""_ms(500.0f)};
	};

	struct TopazRenderData
	{
		tz::gl::RendererHandle renderer = hdk::nullhand;
		tz::gl::RendererHandle final_renderer = hdk::nullhand;
		tz::gl::ResourceHandle vertex_buffer = hdk::nullhand;
		tz::gl::ResourceHandle index_buffer = hdk::nullhand;
		tz::gl::ResourceHandle shader_data_buffer = hdk::nullhand;
	};

	TopazPlatformData* global_platform_data = nullptr;
	TopazRenderData* global_render_data = nullptr;
	tz::GameInfo global_info;
	GameMenuCallbackType game_menu_callback;
	GameBarCallbackType game_bar_callback;

	void imgui_impl_handle_inputs();

	bool imgui_impl_tz_init();
	void imgui_impl_tz_term();
	void imgui_impl_render();
	void imgui_impl_begin_commands();
	void imgui_impl_style_colours_purple();

	ImGuiKey tz_key_to_imgui(tz::KeyCode key_code);
	ImGuiMouseButton tz_btn_to_imgui(tz::MouseButton btn);

	ImTextureID handle_to_texid(tz::gl::ResourceHandle handle)
	{
		return reinterpret_cast<ImTextureID>(static_cast<std::uintptr_t>(static_cast<std::size_t>(static_cast<hdk::hanval>(handle))));
	}

	void initialise([[maybe_unused]] InitInfo info)
	{
		HDK_PROFZONE("tz::dbgui::initialise", 0xFFAA00AA);
		#if HDK_DEBUG
			global_info = info.game_info;
			bool ret = imgui_impl_tz_init();
			hdk::assert(ret, "Failed to initialise tz imgui backend.");
		#endif // HDK_DEBUG
	}

	void terminate()
	{
		HDK_PROFZONE("tz::dbgui::terminate", 0xFFAA00AA);
		#if HDK_DEBUG
			imgui_impl_tz_term();
		#endif // HDK_DEBUG
	}

	void begin_frame()
	{
		HDK_PROFZONE("tz::dbgui::begin_frame", 0xFFAA00AA);
		#if HDK_DEBUG
			ImGuiIO& io = ImGui::GetIO();
			io.DisplaySize = ImVec2
			{
				static_cast<float>(tz::window().get_width()),
				static_cast<float>(tz::window().get_height())
			};
			ImGui::NewFrame();
			imgui_impl_begin_commands();
		#endif // HDK_DEBUG
	}

	void end_frame()
	{
		HDK_PROFZONE("tz::dbgui::end_frame", 0xFFAA00AA);
		#if HDK_DEBUG
			ImGui::EndFrame();
			ImGui::Render();
			imgui_impl_render();

			imgui_impl_handle_inputs();

		#endif //HDK_DEBUG
	}

	GameMenuCallbackType& game_menu()
	{
		return game_menu_callback;
	}

	GameBarCallbackType& game_bar()
	{
		return game_bar_callback;
	}

	bool claims_keyboard()
	{
		#if HDK_DEBUG
			return ImGui::GetIO().WantCaptureKeyboard;
		#endif
		return false;
	}

	bool claims_mouse()
	{
		#if HDK_DEBUG
			return ImGui::GetIO().WantCaptureMouse;
		#endif
		return false;
	}

	struct TopazShaderRenderData
	{
		tz::Mat4 vp;
		std::uint32_t texture_id;
		std::uint32_t index_offset;
		std::uint32_t vertex_offset;
		float pad[1];
	};

	enum class MouseWheelDirection
	{
		Up,
		Down,
		Same
	};

	struct InputDelta
	{
		std::vector<tz::KeyPressInfo> newly_pressed = {};
		std::vector<tz::KeyPressInfo> newly_released = {};
		std::vector<tz::MouseButtonPressInfo> newly_pressed_buttons = {};
		std::vector<tz::MouseButtonPressInfo> newly_released_buttons = {};
		bool mouse_position_changed = false;
		MouseWheelDirection mouse_wheel_dir = MouseWheelDirection::Same;
	};

	InputDelta imgui_impl_get_input_delta()
	{
		#if HDK_DEBUG
			// Get keyboard pressed/released deltas.
			std::span<const tz::KeyPressInfo> before_span = global_platform_data->kb_state.get_pressed_keys();
			std::span<const tz::KeyPressInfo> after_span = tz::window().get_keyboard_state().get_pressed_keys();
			std::vector<tz::KeyPressInfo> before{ before_span.begin(), before_span.end() };
			std::vector<tz::KeyPressInfo> after{ after_span.begin(), after_span.end() };
			std::sort(before.begin(), before.end());
			std::sort(after.begin(), after.end());
			std::vector<tz::KeyPressInfo> newly_pressed;
			std::vector<tz::KeyPressInfo> newly_released;

			std::size_t x = std::max(before.size(), after.size());
			if(x > 0)
			{
				newly_pressed.resize(x);
				newly_released.resize(x);

				auto keypress_comp = [](const tz::KeyPressInfo& a, const tz::KeyPressInfo& b){return static_cast<int>(a.key.code) < static_cast<int>(b.key.code);};
				auto released_end = std::set_difference(before.begin(), before.end(), after.begin(), after.end(), newly_released.begin(), keypress_comp);
				auto pressed_end = std::set_difference(after.begin(), after.end(), before.begin(), before.end(), newly_pressed.begin(), keypress_comp);
				
				newly_pressed.erase(pressed_end, newly_pressed.end());
				newly_released.erase(released_end, newly_released.end());
			}

			// Mouse position and buttons
			bool mouse_moved = tz::window().get_mouse_position_state().get_mouse_position() != global_platform_data->mouse_pos_state.get_mouse_position();
			std::span<const tz::MouseButtonPressInfo> before_buttons = global_platform_data->mouse_button_state.get_pressed_buttons();
			std::span<const tz::MouseButtonPressInfo> after_buttons = tz::window().get_mouse_button_state().get_pressed_buttons();
			std::vector<tz::MouseButtonPressInfo> newly_pressed_buttons;
			std::vector<tz::MouseButtonPressInfo> newly_released_buttons;
			
			float ybefore = global_platform_data->mouse_button_state.get_scroll_offset()[1];
			float ynow = tz::window().get_mouse_button_state().get_scroll_offset()[1];
			MouseWheelDirection mdir = MouseWheelDirection::Same;
			if(ynow > ybefore)
			{
				mdir = MouseWheelDirection::Up;
			}
			else if(ynow < ybefore)
			{
				mdir = MouseWheelDirection::Down;
			}

			x = std::max(before_buttons.size(), after_buttons.size());
			if(x > 0)
			{
				newly_pressed_buttons.resize(x);
				newly_released_buttons.resize(x);
				auto btnpress_comp = [](const tz::MouseButtonPressInfo& a, const tz::MouseButtonPressInfo& b){return static_cast<int>(a.button.button) < static_cast<int>(b.button.button);};
				auto released_end = std::set_difference(before_buttons.begin(), before_buttons.end(), after_buttons.begin(), after_buttons.end(), newly_released_buttons.begin(), btnpress_comp);
				auto pressed_end = std::set_difference(after_buttons.begin(), after_buttons.end(), before_buttons.begin(), before_buttons.end(), newly_pressed_buttons.begin(), btnpress_comp);
				
				newly_pressed_buttons.erase(pressed_end, newly_pressed_buttons.end());
				newly_released_buttons.erase(released_end, newly_released_buttons.end());
			}


			// Update global platform data.
			global_platform_data->kb_state = tz::window().get_keyboard_state();
			global_platform_data->mouse_pos_state = tz::window().get_mouse_position_state();
			global_platform_data->mouse_button_state = tz::window().get_mouse_button_state();

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
		#endif // HDK_DEBUG
		return {};
	}

	void imgui_impl_handle_inputs()
	{
		#if HDK_DEBUG
			ImGuiIO& io = ImGui::GetIO();
			InputDelta delta = imgui_impl_get_input_delta();
			// Pass to imgui.
			for(const auto& press : delta.newly_pressed)
			{
				io.AddKeyEvent(tz_key_to_imgui(press.key.code), true);
				// TODO: Proper text parsing
				io.AddInputCharacter(press.key.representation);
			}
			for(const auto& release : delta.newly_released)
			{
				io.AddKeyEvent(tz_key_to_imgui(release.key.code), false);
			}

			for(const auto& btn_press : delta.newly_pressed_buttons)
			{
				ImGuiMouseButton btn = tz_btn_to_imgui(btn_press.button.button);
				if(btn == ImGuiMouseButton_COUNT)
				{
					continue;
				}
				io.AddMouseButtonEvent(btn, true);
			}
			for(const auto& btn_release : delta.newly_released_buttons)
			{
				ImGuiMouseButton btn = tz_btn_to_imgui(btn_release.button.button);
				if(btn == ImGuiMouseButton_COUNT)
				{
					continue;
				}
				io.AddMouseButtonEvent(btn, false);
			}

			if(delta.mouse_position_changed)
			{
				const auto mpos = static_cast<hdk::vec2>(tz::window().get_mouse_position_state().get_mouse_position());
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
		#endif // HDK_DEBUG
	}

	bool imgui_impl_tz_init()
	{
		ImGui::CreateContext();
		global_platform_data = new TopazPlatformData;
		global_render_data = new TopazRenderData;

		ImGuiIO& io = ImGui::GetIO();
		hdk::assert(io.BackendPlatformUserData == NULL, "Already initialised imgui backend!");
		io.BackendPlatformName = "Topaz";
		io.BackendPlatformUserData = global_platform_data;
		io.BackendRendererName = "tz::gl";
		io.BackendRendererUserData = global_render_data;

		// Make renderer.
		// - Font texture ImageResource.
		unsigned char* font_pixels;
		int font_width, font_height;
		io.Fonts->GetTexDataAsRGBA32(&font_pixels, &font_width, &font_height);
		std::span<unsigned char> font_data{font_pixels, static_cast<std::size_t>(font_width * font_height * 4)};

		// IB and VB have initial size of 1KiB.
		struct Kibibyte{char d[1024];};

		tz::gl::BufferResource vertex_buffer = tz::gl::BufferResource::from_one(Kibibyte{},
		{
			.access = tz::gl::ResourceAccess::DynamicVariable
		});
		tz::gl::BufferResource index_buffer = tz::gl::BufferResource::from_one(Kibibyte{},
		{
			.access = tz::gl::ResourceAccess::DynamicVariable
		});
		tz::gl::BufferResource shader_data_buffer = tz::gl::BufferResource::from_one(TopazShaderRenderData{},
		{
			.access = tz::gl::ResourceAccess::DynamicFixed
		});

		tz::gl::ImageResource font_image = tz::gl::ImageResource::from_memory
		(
			font_data,
			{
				.format = tz::gl::ImageFormat::RGBA32,
				.dimensions = hdk::vec2ui{static_cast<unsigned int>(font_width), static_cast<unsigned int>(font_height)}
			}
		);

		tz::gl::WindowOutput wout{tz::window()};

		tz::gl::RendererInfo rinfo;
		global_render_data->vertex_buffer = rinfo.add_resource(vertex_buffer);
		global_render_data->shader_data_buffer = rinfo.add_resource(shader_data_buffer);
		global_render_data->index_buffer = rinfo.add_resource(index_buffer);
		rinfo.add_resource(font_image);
		rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(dbgui, vertex));
		rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(dbgui, fragment));
		rinfo.set_options({tz::gl::RendererOption::NoClearOutput, tz::gl::RendererOption::NoDepthTesting, tz::gl::RendererOption::NoPresent, tz::gl::RendererOption::RenderWait, tz::gl::RendererOption::Internal});
		rinfo.set_output(wout);
		rinfo.debug_name("ImGui Intermediate Renderer");
		
		global_render_data->renderer = tz::gl::device().create_renderer(rinfo);

		tz::gl::RendererInfo empty;
		empty.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
		empty.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));
		empty.set_options({tz::gl::RendererOption::NoClearOutput, tz::gl::RendererOption::NoDepthTesting, tz::gl::RendererOption::Internal_FinalDebugUIRenderer, tz::gl::RendererOption::Internal});
		empty.debug_name("ImGui Final Renderer");
		global_render_data->final_renderer = tz::gl::device().create_renderer(empty);

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
		HDK_PROFZONE("Dbgui Render", 0xFFAA00AA);

		ImDrawData* draw = ImGui::GetDrawData();
		hdk::assert(draw != nullptr, "Null imgui draw data!");
		hdk::assert(draw->Valid, "Invalid draw data!");

		hdk::assert(global_render_data->renderer != hdk::nullhand, "Null imgui renderer when trying to render!");
		// We have a font texture already.
		tz::gl::Renderer& renderer = tz::gl::device().get_renderer(global_render_data->renderer);
		// We have no idea how big our vertex/index buffers need to be. Let's copy over the data now.
		const auto req_idx_size = static_cast<std::size_t>(draw->TotalIdxCount) * sizeof(ImDrawIdx);
		const auto req_vtx_size = static_cast<std::size_t>(draw->TotalVtxCount) * sizeof(ImDrawVert);
		tz::gl::RendererEditBuilder edit;
		if(renderer.get_resource(global_render_data->index_buffer)->data().size_bytes() <= req_idx_size)
		{
			edit.buffer_resize
			({
				.buffer_handle = global_render_data->index_buffer,
				.size = req_idx_size
			});
		}
		if(renderer.get_resource(global_render_data->vertex_buffer)->data().size_bytes() <= req_vtx_size)
		{
			edit.buffer_resize
			({
				.buffer_handle = global_render_data->vertex_buffer,
				.size = req_vtx_size
			});
		}
		static_assert(sizeof(ImDrawIdx) == sizeof(unsigned int), "Topaz indices must be c++ unsigned ints under-the-hood. ImDrawIdx does not match its size.");
		{
			HDK_PROFZONE("Dbgui Render - IB/VB Resize", 0xFFAA00AA);
			renderer.edit(edit.build());
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
			HDK_PROFZONE("Dbgui Render - Command List Processing", 0xFFAA00AA);
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
			) * tz::view(hdk::vec3{io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f, 0.0f}, {});
			for(const ImDrawCmd& draw_cmd : cmd->CmdBuffer)
			{
				HDK_PROFZONE("Dbgui Render - Single Command Processing", 0xFFAA00AA);
				shader_data.texture_id = static_cast<std::size_t>(reinterpret_cast<std::uintptr_t>(draw_cmd.TextureId));
				shader_data.index_offset = draw_cmd.IdxOffset;
				shader_data.vertex_offset = draw_cmd.VtxOffset;

				tz::gl::IOutput* output = renderer.get_output();
				hdk::assert(output != nullptr, "");
				ImVec2 min = {draw_cmd.ClipRect.x, draw_cmd.ClipRect.y};
				ImVec2 max = {draw_cmd.ClipRect.z - draw_cmd.ClipRect.x, draw_cmd.ClipRect.w - draw_cmd.ClipRect.y};
				output->scissor.offset = static_cast<hdk::vec2ui>(hdk::vec2{min.x, min.y} - hdk::vec2{draw->DisplayPos.x, draw->DisplayPos.y});
				output->scissor.extent = static_cast<hdk::vec2ui>(hdk::vec2{max.x, max.y});
#if TZ_OGL
				output->scissor.offset[1] = io.DisplaySize.y - output->scissor.extent[1] - output->scissor.offset[1];
#endif

				// Do a draw.
				const std::size_t tri_count = draw_cmd.ElemCount / 3;
				if(draw_cmd.UserCallback == nullptr)
				{
					renderer.render(tri_count);
				}
				else
				{
					draw_cmd.UserCallback(cmd, &draw_cmd);
				}
			}
		}
		{
			HDK_PROFZONE("Dbgui Render - Final Pass", 0xFFAA00AA);
			tz::gl::device().get_renderer(global_render_data->final_renderer).render();
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
			hdk::version ver = tz::gl::vk2::vulkan_version;
			ImGui::Text("%u.%u |", ver.major, ver.minor);
		#elif TZ_OGL
			ImGui::Text("OpenGL");	
			ImGui::SameLine();
			hdk::version ver = tz::gl::ogl2::ogl_version;
			ImGui::Text("%u.%u Core Profile |", ver.major, ver.minor);
		#else
			ImGui::Text("Unknown");	
		#endif
		ImGui::SameLine();
		#if HDK_DEBUG
			ImGui::Text("Debug");
		#elif HDK_PROFILE
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
		ImGui::Text("%u jobs", hdk::job_system().size());

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
		//		const tz::gl::vk2::LogicalDevice& ldev = tz::gl::device().vk_get_logical_device();
		//		ImGui::Text("Vulkan Device Extensions:");
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
			if(wnd.is_null())
			{
				ImGui::Text("Detected that `tz::window().is_null()`. Cannot display window diagnostics, you're probably in a really weird state.");
			}
			else
			{
				if(wnd.is_resizeable())
				{
					int xy[] = {static_cast<int>(wnd.get_width()), static_cast<int>(wnd.get_height())};
					if(ImGui::DragInt2("Dimensions", xy, 3, 1, 4192))
					{
						wnd.set_width(static_cast<float>(xy[0]));
						wnd.set_height(static_cast<float>(xy[1]));
					}
				}
				else
				{
					ImGui::Text("Dimensions = {%u, %u} - Fixed-size window", static_cast<unsigned int>(wnd.get_width()), static_cast<unsigned int>(wnd.get_height()));
				}
				ImGui::Text("Minimised = %s", wnd.is_minimised() ? "true" : "false");
				ImGui::Text("Maximised = %s", wnd.is_maximised() ? "true" : "false");
				ImGui::Text("Focused = %s", wnd.is_focused() ? "true" : "false");
			}
			ImGui::End();
		}

	}

	void draw_tz_device_info()
	{
		if(ImGui::Begin("Device", &tab_tz.show_device_info))
		{
			tz::gl::device().dbgui();
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
					hdk::error("Manual debug breakpoint occurred.");
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
				ImGui::SetWindowPos(ImVec2(0, 18), true);
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
		if(global_platform_data->fps_update.done())
		{
			global_platform_data->frame_period = (tz::system_time() - global_platform_data->last_update).millis<float>();
			global_platform_data->fps_update.reset();
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
		}
		colours[ImGuiCol_CheckMark].x *= 2.0f;
		colours[ImGuiCol_CheckMark].y *= 2.0f;
		colours[ImGuiCol_CheckMark].z *= 2.0f;
	}

	ImGuiMouseButton tz_btn_to_imgui(tz::MouseButton button)
	{
		switch(button)
		{
			case tz::MouseButton::Left:
				return ImGuiMouseButton_Left;
			break;
			case tz::MouseButton::Right:
				return ImGuiMouseButton_Right;
			break;
			case tz::MouseButton::Middle:
				return ImGuiMouseButton_Middle;
			break;
			default:
				return ImGuiMouseButton_COUNT;
			break;
		}
	}

	ImGuiKey tz_key_to_imgui(tz::KeyCode key_code)
	{
		switch(key_code)
		{
			case tz::KeyCode::Space:
				return ImGuiKey_Space;
			break;
			case tz::KeyCode::Backspace:
				return ImGuiKey_Backspace;
			break;
			case tz::KeyCode::Apostrophe:
				return ImGuiKey_Apostrophe;
			break;
			case tz::KeyCode::Comma:
				return ImGuiKey_Comma;
			break;
			case tz::KeyCode::Period:
				return ImGuiKey_Period;
			break;
			case tz::KeyCode::Minus:
				return ImGuiKey_Minus;
			break;
			//case Plus:
			//	return ImGuiKey_Plus;
			//break;
			case tz::KeyCode::ForwardSlash:
				return ImGuiKey_Slash;
			break;
			case tz::KeyCode::BackSlash:
				return ImGuiKey_Backslash;
			break;
			case tz::KeyCode::Zero:
				return ImGuiKey_0;
			break;
			case tz::KeyCode::One:
				return ImGuiKey_1;
			break;
			case tz::KeyCode::Two:
				return ImGuiKey_2;
			break;
			case tz::KeyCode::Three:
				return ImGuiKey_3;
			break;
			case tz::KeyCode::Four:
				return ImGuiKey_4;
			break;
			case tz::KeyCode::Five:
				return ImGuiKey_5;
			break;
			case tz::KeyCode::Six:
				return ImGuiKey_6;
			break;
			case tz::KeyCode::Seven:
				return ImGuiKey_7;
			break;
			case tz::KeyCode::Eight:
				return ImGuiKey_8;
			break;
			case tz::KeyCode::Nine:
				return ImGuiKey_9;
			break;
			case tz::KeyCode::Semicolon:
				return ImGuiKey_Semicolon;
			break;
			case tz::KeyCode::Equals:
				return ImGuiKey_Equal;
			break;
			case tz::KeyCode::A:
				return ImGuiKey_A;
			break;
			case tz::KeyCode::B:
				return ImGuiKey_B;
			break;
			case tz::KeyCode::C:
				return ImGuiKey_C;
			break;
			case tz::KeyCode::D:
				return ImGuiKey_D;
			break;
			case tz::KeyCode::E:
				return ImGuiKey_E;
			break;
			case tz::KeyCode::F:
				return ImGuiKey_F;
			break;
			case tz::KeyCode::G:
				return ImGuiKey_G;
			break;
			case tz::KeyCode::H:
				return ImGuiKey_H;
			break;
			case tz::KeyCode::I:
				return ImGuiKey_I;
			break;
			case tz::KeyCode::J:
				return ImGuiKey_J;
			break;
			case tz::KeyCode::K:
				return ImGuiKey_K;
			break;
			case tz::KeyCode::L:
				return ImGuiKey_L;
			break;
			case tz::KeyCode::M:
				return ImGuiKey_M;
			break;
			case tz::KeyCode::N:
				return ImGuiKey_N;
			break;
			case tz::KeyCode::O:
				return ImGuiKey_O;
			break;
			case tz::KeyCode::P:
				return ImGuiKey_P;
			break;
			case tz::KeyCode::Q:
				return ImGuiKey_Q;
			break;
			case tz::KeyCode::R:
				return ImGuiKey_R;
			break;
			case tz::KeyCode::S:
				return ImGuiKey_S;
			break;
			case tz::KeyCode::T:
				return ImGuiKey_T;
			break;
			case tz::KeyCode::U:
				return ImGuiKey_U;
			break;
			case tz::KeyCode::V:
				return ImGuiKey_V;
			break;
			case tz::KeyCode::W:
				return ImGuiKey_W;
			break;
			case tz::KeyCode::X:
				return ImGuiKey_X;
			break;
			case tz::KeyCode::Y:
				return ImGuiKey_Y;
			break;
			case tz::KeyCode::Z:
				return ImGuiKey_Z;
			break;
			case tz::KeyCode::LeftBracket:
				return ImGuiKey_LeftBracket;
			break;
			case tz::KeyCode::RightBracket:
				return ImGuiKey_RightBracket;
			break;
			//case LeftParenthesis:
			//	return ImGuiKey_LeftParenthesis;
			//break;
			//case RightParenthesis:
			//	return ImGuiKey_RightParenthesis;
			//break;
			//case LeftBrace:
			//	return ImGuiKey_LeftBrace;
			//break;
			//case RightBrace:
			//	return ImGuiKey_RightBrace;
			//break;

			default:
				return ImGuiKey_None;
			break;
		}
	}
}
