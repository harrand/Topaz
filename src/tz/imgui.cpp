#include "tz/imgui.hpp"
#include "tz/topaz.hpp"
#include "tz/core/memory.hpp"
#include "tz/gpu/hardware.hpp"
#include "tz/gpu/resource.hpp"
#include "tz/gpu/shader.hpp"
#include "tz/gpu/pass.hpp"
#include <span>
#include <array>

#include ImportedShaderHeader(imgui, vertex)
#include ImportedShaderHeader(imgui, fragment)

namespace tz::detail
{
	// imgui backend initialise.
	struct render_data
	{
		tz::gpu::resource_handle vertex_buffer = tz::nullhand;
		tz::gpu::resource_handle index_buffer = tz::nullhand;

		tz::gpu::resource_handle font_image = tz::nullhand;
		tz::gpu::shader_handle shader = tz::nullhand;

		tz::gpu::pass_handle pass = tz::nullhand;
	} render;

	void impl_write_vertices_and_indices(ImDrawData* draw);

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
			.name = "ImGui Vertex Buffer"
		}));

		render.index_buffer = tz_must(tz::gpu::create_buffer({
			.data = tz::view_bytes(initial_indices),
			.name = "ImGui Index Buffer"
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

		tz::gpu::resource_handle resources[] =
		{
			render.vertex_buffer,
			render.index_buffer,
			render.font_image
		};

		tz::gpu::resource_handle colour_targets[] =
		{
			tz::gpu::window_resource
		};

		render.pass = tz_must(tz::gpu::create_pass({
			.graphics =
			{
				.colour_targets = colour_targets,
				.flags = tz::gpu::graphics_flag::dont_clear | tz::gpu::graphics_flag::no_depth_test,
			},
			.shader = render.shader,
			.resources = resources,
		}));
	}

	void imgui_terminate()
	{
		tz::gpu::destroy_pass(render.pass);
		tz_must(tz::gpu::destroy_resource(render.font_image));
		tz_must(tz::gpu::destroy_resource(render.index_buffer));
		tz_must(tz::gpu::destroy_resource(render.vertex_buffer));
		tz::gpu::destroy_shader(render.shader);
		ImGui::DestroyContext();
	}
	
	// impl bits

	void impl_write_vertices_and_indices(ImDrawData* draw)
	{
		(void)draw;
	}
}