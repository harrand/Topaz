#include "tz/dbgui/dbgui.hpp"
#include "tz/core/assert.hpp"
#include "tz/core/report.hpp"
#include "tz/core/window.hpp"
#include "tz/core/matrix_transform.hpp"
#include "tz/gl/renderer.hpp"
#include "tz/gl/resource.hpp"
#include "tz/gl/imported_shaders.hpp"
#include "imgui.h"

#include <memory>

#include ImportedShaderHeader(dbgui, vertex)
#include ImportedShaderHeader(dbgui, fragment)
#include ImportedShaderHeader(empty, vertex)
#include ImportedShaderHeader(empty, fragment)

namespace tz::dbgui
{
	struct TopazPlatformData
	{
		tz::KeyboardState kb_state;
		tz::MousePositionState mouse_pos_state;
		tz::MouseButtonState mouse_button_state;
	};

	struct TopazRenderData
	{
		std::unique_ptr<tz::gl::Renderer> renderer = nullptr;
		std::unique_ptr<tz::gl::Renderer> final_renderer = nullptr;
		tz::gl::ResourceHandle vertex_buffer = tz::nullhand;
		tz::gl::ResourceHandle index_buffer = tz::nullhand;
		tz::gl::ResourceHandle shader_data_buffer = tz::nullhand;
	};

	TopazPlatformData* global_platform_data = nullptr;
	TopazRenderData* global_render_data = nullptr;
	tz::gl::Device* global_device = nullptr;

	void imgui_impl_handle_inputs();

	bool imgui_impl_tz_init();
	void imgui_impl_tz_term();
	void imgui_impl_render();

	ImGuiKey tz_key_to_imgui(tz::KeyCode key_code);

	ImTextureID handle_to_texid(tz::gl::ResourceHandle handle)
	{
		return reinterpret_cast<ImTextureID>(static_cast<std::uintptr_t>(static_cast<std::size_t>(static_cast<tz::HandleValue>(handle))));
	}

	void initialise([[maybe_unused]] Info info)
	{
		#if TZ_DEBUG
			global_device = info.device;
			bool ret = imgui_impl_tz_init();
			tz_assert(ret, "Failed to initialise tz imgui backend.");
		#endif // TZ_DEBUG
	}

	void terminate()
	{
		#if TZ_DEBUG
			imgui_impl_tz_term();
		#endif // TZ_DEBUG
	}

	void begin_frame()
	{
		#if TZ_DEBUG
			ImGuiIO& io = ImGui::GetIO();
			io.DisplaySize = ImVec2
			{
				static_cast<float>(tz::window().get_width()),
				static_cast<float>(tz::window().get_height())
			};
			ImGui::NewFrame();
		#endif // TZ_DEBUG
	}

	void end_frame()
	{
		#if TZ_DEBUG
			ImGui::EndFrame();
			ImGui::Render();
			imgui_impl_render();

			imgui_impl_handle_inputs();

		#endif //TZ_DEBUG
	}


	struct TopazShaderRenderData
	{
		tz::Mat4 vp;
		std::uint32_t texture_id;
		std::uint32_t index_offset;
		std::uint32_t vertex_offset;
		float pad[1];
	};

	void imgui_impl_handle_inputs()
	{
		#if TZ_DEBUG
			ImGuiIO& io = ImGui::GetIO();
			std::span<const tz::KeyPressInfo> before = global_platform_data->kb_state.get_pressed_keys();
			std::span<const tz::KeyPressInfo> after = tz::window().get_keyboard_state().get_pressed_keys();

			const std::size_t x = std::max(before.size(), after.size());
			if(x > 0)
			{
				std::vector<tz::KeyPressInfo> newly_pressed;
				newly_pressed.resize(x);
				std::vector<tz::KeyPressInfo> newly_released;
				newly_released.resize(x);

				auto keypress_comp = [](const tz::KeyPressInfo& a, const tz::KeyPressInfo& b){return static_cast<int>(a.key.code) < static_cast<int>(b.key.code);};
				auto released_end = std::set_difference(before.begin(), before.end(), after.begin(), after.end(), newly_released.begin(), keypress_comp);
				auto pressed_end = std::set_difference(after.begin(), after.end(), before.begin(), before.end(), newly_pressed.begin(), keypress_comp);
				
				newly_pressed.erase(pressed_end, newly_pressed.end());
				newly_released.erase(released_end, newly_released.end());
				// Pass to imgui.
				for(const auto& press : newly_pressed)
				{
					io.AddKeyEvent(tz_key_to_imgui(press.key.code), true);
				}
				for(const auto& release : newly_released)
				{
					io.AddKeyEvent(tz_key_to_imgui(release.key.code), false);
				}
			}

			if(tz::window().get_mouse_position_state().get_mouse_position() != global_platform_data->mouse_pos_state.get_mouse_position())
			{
				auto mpos = static_cast<tz::Vec2>(tz::window().get_mouse_position_state().get_mouse_position());
				//tz_report("mouse {%.2f, %.2f}", mpos[0], mpos[1]);
				io.AddMousePosEvent(mpos[0], mpos[1]);
			}

			global_platform_data->kb_state = tz::window().get_keyboard_state();
			global_platform_data->mouse_pos_state = tz::window().get_mouse_position_state();
			global_platform_data->mouse_button_state = tz::window().get_mouse_button_state();

		#endif // TZ_DEBUG
	}

	bool imgui_impl_tz_init()
	{
		ImGui::CreateContext();
		global_platform_data = new TopazPlatformData;
		global_render_data = new TopazRenderData;

		ImGuiIO& io = ImGui::GetIO();
		tz_assert(io.BackendPlatformUserData == NULL, "Already initialised imgui backend!");
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

		tz::gl::BufferResource vertex_buffer = tz::gl::BufferResource::from_one(Kibibyte{}, tz::gl::ResourceAccess::DynamicVariable);
		tz::gl::BufferResource index_buffer = tz::gl::BufferResource::from_one(Kibibyte{}, tz::gl::ResourceAccess::DynamicVariable);
		tz::gl::BufferResource shader_data_buffer = tz::gl::BufferResource::from_one(TopazShaderRenderData{}, tz::gl::ResourceAccess::DynamicFixed);

		tz::gl::ImageResource font_image = tz::gl::ImageResource::from_memory(tz::gl::ImageFormat::RGBA32, {static_cast<unsigned int>(font_width), static_cast<unsigned int>(font_height)}, font_data, tz::gl::ResourceAccess::StaticFixed);

		tz::gl::RendererInfo rinfo;
		global_render_data->vertex_buffer = rinfo.add_resource(vertex_buffer);
		global_render_data->shader_data_buffer = rinfo.add_resource(shader_data_buffer);
		global_render_data->index_buffer = rinfo.add_resource(index_buffer);
		rinfo.add_resource(font_image);
		rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(dbgui, vertex));
		rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(dbgui, fragment));
		rinfo.set_options({tz::gl::RendererOption::NoClearOutput, tz::gl::RendererOption::NoDepthTesting, tz::gl::RendererOption::NoPresent});
		
		global_render_data->renderer = std::make_unique<tz::gl::Renderer>(global_device->create_renderer(rinfo));

		tz::gl::RendererInfo empty;
		empty.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
		empty.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));
		empty.set_options({tz::gl::RendererOption::NoClearOutput, tz::gl::RendererOption::NoDepthTesting});
		global_render_data->final_renderer = std::make_unique<tz::gl::Renderer>(global_device->create_renderer(empty));

		io.Fonts->SetTexID(0);

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
		ImDrawData* draw = ImGui::GetDrawData();
		tz_assert(draw != nullptr, "Null imgui draw data!");
		tz_assert(draw->Valid, "Invalid draw data!");

		tz_assert(global_render_data->renderer != nullptr, "Null imgui renderer when trying to render!");
		// We have a font texture already.
		tz::gl::Renderer& renderer = *(global_render_data->renderer);
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
		renderer.edit(edit.build());
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
			) * tz::view(tz::Vec3{io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f, 0.0f}, {});
			for(const ImDrawCmd& draw_cmd : cmd->CmdBuffer)
			{
				shader_data.texture_id = static_cast<std::size_t>(reinterpret_cast<std::uintptr_t>(draw_cmd.TextureId));
				shader_data.index_offset = draw_cmd.IdxOffset;
				shader_data.vertex_offset = draw_cmd.VtxOffset;

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
		global_render_data->final_renderer->render();
	}

	ImGuiKey tz_key_to_imgui(tz::KeyCode key_code)
	{
		switch(key_code)
		{
			using enum tz::KeyCode;
			case Space:
				return ImGuiKey_Space;
			break;
			case Backspace:
				return ImGuiKey_Backspace;
			break;
			case Apostrophe:
				return ImGuiKey_Apostrophe;
			break;
			case Comma:
				return ImGuiKey_Comma;
			break;
			case Period:
				return ImGuiKey_Period;
			break;
			case Minus:
				return ImGuiKey_Minus;
			break;
			//case Plus:
			//	return ImGuiKey_Plus;
			//break;
			case ForwardSlash:
				return ImGuiKey_Slash;
			break;
			case BackSlash:
				return ImGuiKey_Backslash;
			break;
			case Zero:
				return ImGuiKey_0;
			break;
			case One:
				return ImGuiKey_1;
			break;
			case Two:
				return ImGuiKey_2;
			break;
			case Three:
				return ImGuiKey_3;
			break;
			case Four:
				return ImGuiKey_4;
			break;
			case Five:
				return ImGuiKey_5;
			break;
			case Six:
				return ImGuiKey_6;
			break;
			case Seven:
				return ImGuiKey_7;
			break;
			case Eight:
				return ImGuiKey_8;
			break;
			case Nine:
				return ImGuiKey_9;
			break;
			case Semicolon:
				return ImGuiKey_Semicolon;
			break;
			case Equals:
				return ImGuiKey_Equal;
			break;
			case A:
				return ImGuiKey_A;
			break;
			case B:
				return ImGuiKey_B;
			break;
			case C:
				return ImGuiKey_C;
			break;
			case D:
				return ImGuiKey_D;
			break;
			case E:
				return ImGuiKey_E;
			break;
			case F:
				return ImGuiKey_F;
			break;
			case G:
				return ImGuiKey_G;
			break;
			case H:
				return ImGuiKey_H;
			break;
			case I:
				return ImGuiKey_I;
			break;
			case J:
				return ImGuiKey_J;
			break;
			case K:
				return ImGuiKey_K;
			break;
			case L:
				return ImGuiKey_L;
			break;
			case M:
				return ImGuiKey_M;
			break;
			case N:
				return ImGuiKey_N;
			break;
			case O:
				return ImGuiKey_O;
			break;
			case P:
				return ImGuiKey_P;
			break;
			case Q:
				return ImGuiKey_Q;
			break;
			case R:
				return ImGuiKey_R;
			break;
			case S:
				return ImGuiKey_S;
			break;
			case T:
				return ImGuiKey_T;
			break;
			case U:
				return ImGuiKey_U;
			break;
			case V:
				return ImGuiKey_V;
			break;
			case W:
				return ImGuiKey_W;
			break;
			case X:
				return ImGuiKey_X;
			break;
			case Y:
				return ImGuiKey_Y;
			break;
			case Z:
				return ImGuiKey_Z;
			break;
			
			case LeftBracket:
				return ImGuiKey_LeftBracket;
			break;
			case RightBracket:
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
