#include "tz/core/tz.hpp"
#include "tz/core/assert.hpp"
#include "tz/gl/renderer.hpp"
#include "tz/gl/resource.hpp"
#include "tz/gl/device.hpp"
#include "tz/gl/imported_shaders.hpp"

#include ImportedShaderHeader(empty, vertex)
#include ImportedShaderHeader(empty, fragment)
#include ImportedShaderHeader(empty, compute)

#define TESTFUNC_BEGIN(n) void n(){ std::size_t internal_rcount = tz::gl::device().renderer_count();
#define TESTFUNC_END ;tz_assert(tz::gl::device().renderer_count() == internal_rcount, "Detected test function that ended with a different number of renderers (%zu) than it started with (%zu).", tz::gl::device().renderer_count(), internal_rcount);}

//--------------------------------------------------------------------------------------------------
TESTFUNC_BEGIN(create_destroy_empty_renderer)
	// Both graphics and compute renderers.
	// Start with graphics.
	tz::gl::RendererInfo rinfo;
	rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
	rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));
	tz::gl::RendererHandle rh = tz::gl::device().create_renderer(rinfo);

	tz_assert(!tz::gl::device().get_renderer(rh).is_null(), "Empty renderer (Graphics) is considered the null renderer.");
	// Now compute.
	tz::gl::RendererInfo cinfo;
	cinfo.shader().set_shader(tz::gl::ShaderStage::Compute, ImportedShaderSource(empty, compute));
	tz::gl::RendererHandle ch = tz::gl::device().create_renderer(rinfo);

	tz_assert(!tz::gl::device().get_renderer(ch).is_null(), "Empty renderer (Compute) is considered the null renderer.");

	// Try rendering with them both. Should not crash.
	tz::gl::device().get_renderer(rh).render();
	tz::gl::device().get_renderer(ch).render();

	tz::gl::device().destroy_renderer(rh);
	tz::gl::device().destroy_renderer(ch);
TESTFUNC_END

//--------------------------------------------------------------------------------------------------
TESTFUNC_BEGIN(rendereredit_bufferresize)
	// Create a renderer with buffer containing {1.0f}, then resize it to {2.0f, 3.0f}
	tz::gl::RendererInfo rinfo;
	rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
	rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));
	tz::gl::ResourceHandle bh = rinfo.add_resource(tz::gl::BufferResource::from_one(1.0f,
	{
		.access = tz::gl::ResourceAccess::DynamicVariable
	}));
	tz::gl::RendererHandle rh = tz::gl::device().create_renderer(rinfo);

	// Try rendering and make sure it doesn't crash.
	tz::gl::device().get_renderer(rh).render();

	// Do the resize.
	{
		tz::gl::Renderer& ren = tz::gl::device().get_renderer(rh);
		tz_assert(ren.get_resource(bh)->data().size_bytes() == sizeof(float) * 1, "Buffer had unexpected size prior to bufferresize edit. Expected %d bytes, got %zu", sizeof(float) * 1, ren.get_resource(bh)->data().size_bytes());

		ren.edit
		(
			tz::gl::RendererEditBuilder{}
			.buffer_resize
			({
				.buffer_handle = bh,
				.size = sizeof(float) * 2
			})
			.build()
		);

		tz_assert(ren.get_resource(bh)->data().size_bytes() == sizeof(float) * 2, "Buffer had unexpected size after bufferresize edit. Expected %d bytes, got %zu", sizeof(float) * 2, ren.get_resource(bh)->data().size_bytes());
	}

	// Try rendering and make sure it doesn't crash.
	tz::gl::device().get_renderer(rh).render();

	tz::gl::device().destroy_renderer(rh);
TESTFUNC_END

//--------------------------------------------------------------------------------------------------
int main()
{
	tz::initialise
	({
		.name = "tz_renderer_test",
		.flags = {tz::ApplicationFlag::HiddenWindow}
	});
	{
		create_destroy_empty_renderer();

		rendereredit_bufferresize();
	}
	tz::terminate();
}
