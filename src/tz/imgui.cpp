#include "tz/imgui.hpp"
#include "tz/core/trs.hpp"
#include "tz/topaz.hpp"
#include "tz/core/memory.hpp"
#include "tz/core/matrix.hpp"
#include "tz/gpu/hardware.hpp"
#include "tz/gpu/resource.hpp"
#include "tz/gpu/shader.hpp"
#include "tz/gpu/pass.hpp"
#include <span>
#include <array>
#include <vector>

#include ImportedShaderHeader(imgui, vertex)
#include ImportedShaderHeader(imgui, fragment)

namespace tz
{
	namespace detail
	{
		struct shader_data
		{
			tz::m4f vp = tz::m4f::iden();
			std::uint32_t index_offset = 0;
			std::uint32_t vertex_offset = 0;
			std::uint32_t texture_id = 0;
		};
		// imgui backend initialise.
		struct render_data
		{
			tz::gpu::resource_handle vertex_buffer = tz::nullhand;
			tz::gpu::resource_handle index_buffer = tz::nullhand;

			tz::gpu::resource_handle font_image = tz::nullhand;
			tz::gpu::shader_handle shader = tz::nullhand;

			std::vector<tz::gpu::resource_handle> data_buffers = {};

			std::vector<tz::gpu::pass_handle> passes = {};
			tz::gpu::graph_handle graph = tz::nullhand;
		} render;

		void impl_write_vertices_and_indices(ImDrawData* draw);
		void impl_push_pass();
		void impl_pop_pass();
		void impl_on_render(tz::gpu::graph_handle graph);

		void imgui_initialise()
		{
			ImGui::CreateContext();
			if(!gpu::hardware_in_use())
			{
				gpu::use_hardware(gpu::find_best_hardware());
			}

			ImGuiIO& io = ImGui::GetIO();
			io.BackendPlatformName = "Topaz";
			io.BackendRendererName = "Topaz";

			// make font texture resource.
			unsigned char* fontptr;
			int fontw, fonth;
			io.Fonts->GetTexDataAsRGBA32(&fontptr, &fontw, &fonth);
			std::span<unsigned char> fontdata{fontptr, static_cast<std::size_t>(fontw * fonth * 4u)};

			// reserve initially 1024 vertices (and indices).
			static_assert(sizeof(tz::gpu::index_t) == sizeof(ImDrawIdx));
			std::array<tz::gpu::index_t, 1024> initial_indices;
			std::array<ImDrawVert, 1024> initial_vertices;
			render.vertex_buffer = tz_must(tz::gpu::create_buffer({
				.data = tz::view_bytes(initial_vertices),
				.name = "ImGui Vertex Buffer",
				.flags = tz::gpu::buffer_flag::dynamic_access
			}));

			render.index_buffer = tz_must(tz::gpu::create_buffer({
				.data = tz::view_bytes(initial_indices),
				.name = "ImGui Index Buffer",
				.flags = tz::gpu::buffer_flag::dynamic_access
			}));

			render.font_image = tz_must(tz::gpu::create_image
			({
				.width = static_cast<unsigned int>(fontw),
				.height = static_cast<unsigned int>(fonth),
				.data = std::as_bytes(fontdata),
				.name = "ImGui Font image"
			}));

			render.shader = tz_must(tz::gpu::create_graphics_shader(
				ImportedShaderSource(imgui, vertex),
				ImportedShaderSource(imgui, fragment)
			));

			// empty graph for now, because we dont know what imgui wants to draw until we end the frame.
			render.graph = tz::gpu::create_graph("ImGui Render Graph");
			tz::gpu::graph_set_execute_callback(render.graph, impl_on_render);
		}

		void imgui_terminate()
		{
			for(tz::gpu::pass_handle pass : render.passes)
			{
				tz::gpu::destroy_pass(pass);
			}
			tz_must(tz::gpu::destroy_resource(render.font_image));
			tz_must(tz::gpu::destroy_resource(render.index_buffer));
			tz_must(tz::gpu::destroy_resource(render.vertex_buffer));
			tz::gpu::destroy_shader(render.shader);
			ImGui::DestroyContext();
		}

		// impl bits

		void impl_on_render(gpu::graph_handle graph)
		{
			(void)graph;
			// we have a graph that could be completely wrong.
			ImGui::Render();
			ImDrawData* draws = ImGui::GetDrawData();
			if(draws == nullptr)
			{
				return;
			}

			while(std::cmp_greater(draws->CmdListsCount, render.passes.size()))
			{
				impl_push_pass();
			}
			while(std::cmp_less(draws->CmdListsCount, render.passes.size()))
			{
				impl_pop_pass();
			}
			impl_write_vertices_and_indices(draws);
		}

		void impl_write_vertices_and_indices(ImDrawData* draw)
		{
			ImGuiIO io = ImGui::GetIO();

			const auto req_idx_size = static_cast<std::size_t>(draw->TotalIdxCount) * sizeof(ImDrawIdx);
			const auto req_vtx_size = static_cast<std::size_t>(draw->TotalVtxCount) * sizeof(ImDrawVert);

			if(tz::gpu::resource_size(render.vertex_buffer) < req_vtx_size)
			{
				tz::gpu::buffer_resize(render.vertex_buffer, req_vtx_size);
			}
			if(tz::gpu::resource_size(render.index_buffer) < req_idx_size)
			{
				tz::gpu::buffer_resize(render.index_buffer, req_idx_size);
			}
			std::size_t vtx_cursor = 0;
			std::size_t idx_cursor = 0;

			std::vector<ImDrawVert> new_vertices(draw->TotalVtxCount);
			std::vector<ImDrawIdx> new_indices(draw->TotalIdxCount);
			for(std::size_t i = 0; std::cmp_less(i, draw->CmdListsCount); i++)
			{
				const ImDrawList* cmd = draw->CmdLists[i];

				auto data = *reinterpret_cast<const shader_data*>(tz::gpu::resource_read(render.data_buffers[i]).data());
				data.vertex_offset = vtx_cursor;
				data.index_offset = idx_cursor;
				data.vp = tz::matrix_ortho(
					-io.DisplaySize.x * 0.5f,
					io.DisplaySize.x * 0.5f,
					-io.DisplaySize.y * 0.5f,
					io.DisplaySize.y * 0.5f,
					-0.1f,
					0.1f
				);
				tz::gpu::resource_write(render.data_buffers[i], std::as_bytes(std::span<const shader_data>(&data, 1)));

				std::copy(cmd->VtxBuffer.Data, cmd->VtxBuffer.Data + cmd->VtxBuffer.Size, new_vertices.data() + vtx_cursor);
				vtx_cursor += cmd->VtxBuffer.Size;

				std::copy(cmd->IdxBuffer.Data, cmd->IdxBuffer.Data + cmd->IdxBuffer.Size, new_indices.data() + idx_cursor);
				idx_cursor += cmd->IdxBuffer.Size;

				tz::gpu::pass_set_triangle_count(render.passes[i], cmd->IdxBuffer.Size / 3);
			}

			tz::gpu::resource_write(render.vertex_buffer, tz::view_bytes(new_vertices));
			tz::gpu::resource_write(render.index_buffer, tz::view_bytes(new_indices));
		}

		void impl_push_pass()
		{
			shader_data data;
			std::span<const shader_data> span(&data, 1);
			std::string data_buf_name = std::format("ImGui Data Buffer {}", render.data_buffers.size());
			render.data_buffers.push_back(tz_must(tz::gpu::create_buffer
			({
				.data = std::as_bytes(span),
				.name = data_buf_name.c_str(),
				.flags = tz::gpu::buffer_flag::dynamic_access,
			})));
			tz::gpu::resource_handle resources[] =
			{
				render.vertex_buffer,
				render.index_buffer,
				render.data_buffers[render.passes.size()],
				render.font_image
			};

			tz::gpu::resource_handle colour_targets[] =
			{
				tz::gpu::window_resource
			};

			std::string pass_name = std::format("ImGui Pass {}", render.passes.size());

			render.passes.push_back(tz_must(tz::gpu::create_pass({
				.graphics =
				{
					.colour_targets = colour_targets,
					.culling = tz::gpu::cull::none,
					.flags = tz::gpu::graphics_flag::dont_clear | tz::gpu::graphics_flag::no_depth_test,
				},
				.shader = render.shader,
				.resources = resources,
				.name = pass_name.c_str()
			})));
			
			auto sz = render.passes.size();
			std::vector<tz::gpu::pass_handle> deps;
			deps.reserve(1);
			if(sz >= 2)
			{
				deps.push_back(render.passes[sz - 2]);
			}
			tz::gpu::graph_add_pass(render.graph, render.passes[sz - 1], deps);
		}

		void impl_pop_pass()
		{
			tz::gpu::destroy_resource(render.data_buffers.back());
			render.data_buffers.pop_back();
			tz::gpu::destroy_pass(render.passes.back());
			render.passes.pop_back();
			tz_error("todo: implement graph remove pass");
		}
	}

	tz::gpu::graph_handle imgui_render_graph()
	{
		return detail::render.graph;
	}
		
}