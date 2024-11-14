#include "imgui.h"
#include "tz/gpu/graph.hpp"

namespace tz::detail
{
	void imgui_initialise();
	void imgui_terminate();

	tz::gpu::graph_handle imgui_render_graph();
}