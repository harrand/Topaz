#include "tz/dbgui/dbgui.hpp"
#include "tz/core/assert.hpp"
#include "tz/core/window.hpp"
#include "tz/core/matrix_transform.hpp"
#include "tz/gl/renderer.hpp"
#include "tz/gl/resource.hpp"
#include "tz/gl/imported_shaders.hpp"
#include "imgui.h"

#include <memory>

#include ImportedShaderHeader(dbgui, vertex)
#include ImportedShaderHeader(dbgui, fragment)

namespace tz::dbgui
{
	tz::gl::Device* global_device = nullptr;

	bool imgui_impl_tz_init();
	void imgui_impl_tz_term();
	void imgui_impl_render();
	ImTextureID handle_to_texid(tz::gl::ResourceHandle handle)
	{
		return reinterpret_cast<ImTextureID>(static_cast<std::uintptr_t>(static_cast<std::size_t>(static_cast<tz::HandleValue>(handle))));
	}

	void initialise(Info info)
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
		#endif //TZ_DEBUG
	}

	struct TopazPlatformData
	{

	};

	struct TopazRenderData
	{
		std::unique_ptr<tz::gl::Renderer> renderer = nullptr;
		tz::gl::ResourceHandle vertex_buffer = tz::nullhand;
		tz::gl::ResourceHandle index_buffer = tz::nullhand;
		tz::gl::ResourceHandle shader_data_buffer = tz::nullhand;
	};

	TopazPlatformData* global_platform_data = nullptr;
	TopazRenderData* global_render_data = nullptr;

	struct TopazShaderRenderData
	{
		tz::Mat4 vp;
		std::uint32_t texture_id;
		float pad[3];
	};

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
		tz::gl::BufferResource index_buffer = tz::gl::BufferResource::from_one(Kibibyte{}, tz::gl::ResourceAccess::DynamicVariable, {tz::gl::ResourceFlag::IndexBuffer});
		tz::gl::BufferResource shader_data_buffer = tz::gl::BufferResource::from_one(TopazShaderRenderData{}, tz::gl::ResourceAccess::DynamicFixed);

		tz::gl::ImageResource font_image = tz::gl::ImageResource::from_memory(tz::gl::ImageFormat::RGBA32, {static_cast<unsigned int>(font_width), static_cast<unsigned int>(font_height)}, font_data, tz::gl::ResourceAccess::StaticFixed);

		tz::gl::RendererInfo rinfo;
		global_render_data->vertex_buffer = rinfo.add_resource(vertex_buffer);
		global_render_data->shader_data_buffer = rinfo.add_resource(shader_data_buffer);
		global_render_data->index_buffer = rinfo.add_resource(index_buffer);
		tz::gl::ResourceHandle font_image_handle = rinfo.add_resource(font_image);
		rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(dbgui, vertex));
		rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(dbgui, fragment));
		rinfo.set_options({tz::gl::RendererOption::NoClearOutput, tz::gl::RendererOption::NoDepthTesting});
		
		global_render_data->renderer = std::make_unique<tz::gl::Renderer>(global_device->create_renderer(rinfo));

		io.Fonts->SetTexID(handle_to_texid(font_image_handle));

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
		auto indices = renderer.get_resource(global_render_data->index_buffer)->data();
		auto vertices = renderer.get_resource(global_render_data->vertex_buffer)->data();
		renderer.edit(edit.build());
		// Copy over all the vertex and index data.
		for(std::size_t n = 0; std::cmp_less(n, draw->CmdListsCount); n++)
		{
			std::memset(indices.data(), 0, indices.size_bytes());
			std::memset(vertices.data(), 0, vertices.size_bytes());
			const ImDrawList* cmd = draw->CmdLists[n];
			// So far we only support command buffer with a single render.
			tz_assert(cmd->CmdBuffer.Size == 1, "ImDrawList command buffer had more than 1 command. Support for this is not yet implemented.");
			ImDrawCmd draw_cmd = cmd->CmdBuffer[0];
			// Copy over the vertex and index data.
			std::memcpy(indices.data(), cmd->IdxBuffer.Data, cmd->IdxBuffer.Size * sizeof(ImDrawIdx));
			std::memcpy(vertices.data(), cmd->VtxBuffer.Data, cmd->VtxBuffer.Size * sizeof(ImDrawVert));
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
			);
			shader_data.texture_id = 0u;

			// Do a draw.
			const std::size_t tri_count = draw_cmd.ElemCount / 3;
			renderer.render(tri_count);
		}
	}
}
