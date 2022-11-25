#include "tz/core/tz.hpp"
#include "hdk/debug.hpp"
#include "hdk/debug.hpp"
#include "tz/gl/renderer.hpp"
#include "tz/gl/resource.hpp"
#include "tz/gl/device.hpp"
#include "tz/gl/draw.hpp"
#include "tz/gl/imported_shaders.hpp"

#include ImportedShaderHeader(empty, vertex)
#include ImportedShaderHeader(empty, fragment)
#include ImportedShaderHeader(empty, compute)

#define TESTFUNC_BEGIN(n) void n(){ std::size_t internal_rcount = tz::gl::device().renderer_count();
#define TESTFUNC_END ;hdk::assert(tz::gl::device().renderer_count() == internal_rcount, "Detected test function that ended with a different number of renderers (%zu) than it started with (%zu).", tz::gl::device().renderer_count(), internal_rcount);}

bool image_resources_supported()
{
	// TODO: Better support for OGL non-bindless textures. These fail for bindful OGL textures.
	#if TZ_OGL
		return tz::gl::ogl2::supports_bindless_textures();
	#else
		return true;
	#endif
}

//--------------------------------------------------------------------------------------------------
TESTFUNC_BEGIN(create_destroy_empty_renderer)
	// Both graphics and compute renderers.
	// Start with graphics.
	tz::gl::RendererInfo rinfo;
	rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
	rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));
	tz::gl::RendererHandle rh = tz::gl::device().create_renderer(rinfo);

	hdk::assert(!tz::gl::device().get_renderer(rh).is_null(), "Empty renderer (Graphics) is considered the null renderer.");
	// Now compute.
	tz::gl::RendererInfo cinfo;
	cinfo.shader().set_shader(tz::gl::ShaderStage::Compute, ImportedShaderSource(empty, compute));
	tz::gl::RendererHandle ch = tz::gl::device().create_renderer(rinfo);

	hdk::assert(!tz::gl::device().get_renderer(ch).is_null(), "Empty renderer (Compute) is considered the null renderer.");

	// Try rendering with them both. Should not crash.
	tz::gl::device().get_renderer(rh).render();
	tz::gl::device().get_renderer(ch).render();

	tz::gl::device().destroy_renderer(rh);
	tz::gl::device().destroy_renderer(ch);
TESTFUNC_END

//--------------------------------------------------------------------------------------------------
TESTFUNC_BEGIN(renderer_resource_reference_buffer)
	// Create 2 renderers. One with a buffer resource, the other referencing that buffer resource.
	constexpr float expected_value = 69.0f;
	tz::gl::RendererInfo rinfo1;
	rinfo1.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
	rinfo1.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));
	tz::gl::ResourceHandle r1bh = rinfo1.add_resource(tz::gl::BufferResource::from_one(expected_value));
	tz::gl::RendererHandle r1h = tz::gl::device().create_renderer(rinfo1);


	tz::gl::RendererInfo rinfo2;
	rinfo2.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
	rinfo2.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));
	tz::gl::ResourceHandle r2bh = rinfo2.ref_resource(r1h, r1bh);

	tz::gl::RendererHandle r2h = tz::gl::device().create_renderer(rinfo2);

	// Try rendering with them as they shouldn't crash.
	tz::gl::device().get_renderer(r1h).render();
	tz::gl::device().get_renderer(r2h).render();

	// Make sure the buffer resource reference contains the expected value.
	{
		float actual = tz::gl::device().get_renderer(r2h).get_resource(r2bh)->data_as<float>().front();
		hdk::assert(actual == expected_value, "Renderer with buffer resource reference against buffer data {%g} did not have the same data, it had {%g}", expected_value, actual);
	}
	
	tz::gl::device().destroy_renderer(r2h);
	tz::gl::device().destroy_renderer(r1h);
TESTFUNC_END

//--------------------------------------------------------------------------------------------------
TESTFUNC_BEGIN(renderer_resource_reference_image)
	if(!image_resources_supported())
	{
		hdk::report("Test skipped due to lack of support for image resources.");	
		return;
	}

	// Create 2 renderers. One with an image resource, the other referencing that image resource.
	constexpr std::uint32_t expected_value = 0xff0000ff;
	tz::gl::RendererInfo rinfo1;
	rinfo1.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
	rinfo1.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));
	tz::gl::ResourceHandle r1ih = rinfo1.add_resource(tz::gl::ImageResource::from_memory({expected_value},
	{
		.format = tz::gl::ImageFormat::RGBA32,
		.dimensions = {1u, 1u},
	}));
	tz::gl::RendererHandle r1h = tz::gl::device().create_renderer(rinfo1);


	tz::gl::RendererInfo rinfo2;
	rinfo2.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
	rinfo2.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));
	tz::gl::ResourceHandle r2ih = rinfo2.ref_resource(r1h, r1ih);

	tz::gl::RendererHandle r2h = tz::gl::device().create_renderer(rinfo2);

	// Try rendering with them as they shouldn't crash.
	tz::gl::device().get_renderer(r1h).render();
	tz::gl::device().get_renderer(r2h).render();

	// Make sure the image resource reference contains the expected value.
	{
		std::uint32_t actual = tz::gl::device().get_renderer(r2h).get_resource(r2ih)->data_as<std::uint32_t>().front();
		hdk::assert(actual == expected_value, "Renderer with buffer resource reference against buffer data {%g} did not have the same data, it had {%g}", expected_value, actual);
	}
	
	tz::gl::device().destroy_renderer(r2h);
	tz::gl::device().destroy_renderer(r1h);
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
		hdk::assert(ren.get_resource(bh)->data().size_bytes() == sizeof(float) * 1, "Buffer had unexpected size prior to bufferresize edit. Expected %d bytes, got %zu", sizeof(float) * 1, ren.get_resource(bh)->data().size_bytes());

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

		hdk::assert(ren.get_resource(bh)->data().size_bytes() == sizeof(float) * 2, "Buffer had unexpected size after bufferresize edit. Expected %d bytes, got %zu", sizeof(float) * 2, ren.get_resource(bh)->data().size_bytes());
	}

	// Try rendering and make sure it doesn't crash.
	tz::gl::device().get_renderer(rh).render();

	tz::gl::device().destroy_renderer(rh);
TESTFUNC_END

//--------------------------------------------------------------------------------------------------
TESTFUNC_BEGIN(rendereredit_imageresize)
	if(!image_resources_supported())
	{
		hdk::report("Test skipped due to lack of support for image resources.");	
		return;
	}
	// Create a renderer with image of dimensions {1, 1} and then resize to {2, 2}
	tz::gl::RendererInfo rinfo;
	rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
	rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));

	constexpr tz::Vec2ui old_dims{1u, 1u};
	constexpr tz::Vec2ui new_dims{2u, 2u};

	tz::gl::ResourceHandle ih = rinfo.add_resource(tz::gl::ImageResource::from_uninitialised
	({
		.format = tz::gl::ImageFormat::RGBA32,
		.dimensions = old_dims,
		.access = tz::gl::ResourceAccess::DynamicVariable
	}));
	tz::gl::RendererHandle rh = tz::gl::device().create_renderer(rinfo);

	// Try rendering and make sure it doesn't crash.
	tz::gl::device().get_renderer(rh).render();

	// Do the resize.
	{
		tz::gl::Renderer& ren = tz::gl::device().get_renderer(rh);
		const tz::gl::ImageResource* ires = static_cast<const tz::gl::ImageResource*>(ren.get_resource(ih));

		hdk::assert(ires->get_dimensions() == old_dims, "Image had unexpected dimensions prior to imageresize edit. Expected {%u, %u}, got {%u, %u}", old_dims[0], old_dims[1], ires->get_dimensions()[0], ires->get_dimensions()[1]);

		ren.edit
		(
			tz::gl::RendererEditBuilder{}
			.image_resize
			({
				.image_handle = ih,
				.dimensions = new_dims
			})
			.build()
		);

		hdk::assert(ires->get_dimensions() == new_dims, "Image had unexpected dimensions prior to imageresize edit. Expected {%u, %u}, got {%u, %u}", new_dims[0], new_dims[1], ires->get_dimensions()[0], ires->get_dimensions()[1]);

	}

	// Try rendering and make sure it doesn't crash.
	tz::gl::device().get_renderer(rh).render();

	tz::gl::device().destroy_renderer(rh);
TESTFUNC_END

//--------------------------------------------------------------------------------------------------
TESTFUNC_BEGIN(rendereredit_resourcewrite_buffer)
	// Create a renderer with buffer containing {1.0f, 2.0f, 3.0f}, then write {4.0f, 5.0f, 6.0f} into it
	constexpr std::array<float, 3> old_data{1.0f, 2.0f, 3.0f};
	constexpr std::array<float, 3> new_data{4.0f, 5.0f, 6.0f};

	tz::gl::RendererInfo rinfo;
	rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
	rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));
	tz::gl::ResourceHandle bh = rinfo.add_resource(tz::gl::BufferResource::from_many(old_data,
	{
		.access = tz::gl::ResourceAccess::DynamicFixed
		// TODO: Test should pass even if StaticFixed (right now because component has no mapped data, the default resource data is unchanged so the asserts will fail)
	}));
	tz::gl::RendererHandle rh = tz::gl::device().create_renderer(rinfo);

	// Try rendering and make sure it doesn't crash.
	tz::gl::device().get_renderer(rh).render();

	// Do the write.
	{
		tz::gl::Renderer& ren = tz::gl::device().get_renderer(rh);
		{
			std::span<const float> bufdata = ren.get_resource(bh)->data_as<const float>();
			hdk::assert(std::equal(bufdata.begin(), bufdata.end(), old_data.begin()), "Buffer data did not match expected values prior to resource write.");
		}
		ren.edit
		(
			tz::gl::RendererEditBuilder{}
			.write
			({
				.resource = bh,
				.data = std::as_bytes(std::span<const float>(new_data)),
			})
			.build()
		);
		{
			std::span<const float> bufdata = ren.get_resource(bh)->data_as<const float>();
			hdk::assert(std::equal(bufdata.begin(), bufdata.end(), new_data.begin()), "Buffer data did not match expected values prior to resource write.");
		}

	}

	// Try rendering and make sure it doesn't crash.
	tz::gl::device().get_renderer(rh).render();
	
	tz::gl::device().destroy_renderer(rh);
TESTFUNC_END

//--------------------------------------------------------------------------------------------------
TESTFUNC_BEGIN(rendereredit_resourcewrite_image)
	if(!image_resources_supported())
	{
		hdk::report("Test skipped due to lack of support for image resources.");	
		return;
	}
	// Create a renderer with image of dimensions {1, 1}, colour black.
	tz::gl::RendererInfo rinfo;
	rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
	rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));

	constexpr std::uint32_t black_pixel = 0x000000ff;
	constexpr std::uint32_t white_pixel = 0xffffffff;

	tz::gl::ResourceHandle ih = rinfo.add_resource(tz::gl::ImageResource::from_memory
	(
	{
		black_pixel
	},
	{
		.format = tz::gl::ImageFormat::RGBA32,
		.dimensions = {2u, 2u},
		.access = tz::gl::ResourceAccess::DynamicFixed
	}));
	tz::gl::RendererHandle rh = tz::gl::device().create_renderer(rinfo);

	// Try rendering and make sure it doesn't crash.
	tz::gl::device().get_renderer(rh).render();

	// Do the write.
	{
		tz::gl::Renderer& ren = tz::gl::device().get_renderer(rh);
		hdk::assert(ren.get_resource(ih)->data_as<std::uint32_t>().front() == black_pixel, "Expected colour %x, got %x", black_pixel, ren.get_resource(ih)->data_as<std::uint32_t>().front());
		ren.edit
		(
			tz::gl::RendererEditBuilder{}
			.write
			({
				.resource = ih,
				.data = std::as_bytes(std::span<const std::uint32_t>{&white_pixel, 1})
			})
			.build()
		);

		hdk::assert(ren.get_resource(ih)->data_as<std::uint32_t>().front() == white_pixel, "Expected colour %x, got %x", white_pixel, ren.get_resource(ih)->data_as<std::uint32_t>().front());
	}

	// Try rendering and make sure it doesn't crash.
	tz::gl::device().get_renderer(rh).render();

	tz::gl::device().destroy_renderer(rh);
TESTFUNC_END

//--------------------------------------------------------------------------------------------------
TESTFUNC_BEGIN(rendereredit_computeconfig)
	tz::gl::RendererInfo rinfo;
	rinfo.shader().set_shader(tz::gl::ShaderStage::Compute, ImportedShaderSource(empty, compute));

	tz::gl::RendererHandle ch = tz::gl::device().create_renderer(rinfo);

	// Try rendering and make sure it doesn't crash.
	tz::gl::device().get_renderer(ch).render();
	{
		tz::gl::Renderer& ren = tz::gl::device().get_renderer(ch);
		ren.edit
		(
			tz::gl::RendererEditBuilder{}
			.compute
			({
				.kernel = {4u, 4u, 4u}
			})
			.build()
		);
	}
	// Try rendering and make sure it doesn't crash.
	tz::gl::device().get_renderer(ch).render();

	tz::gl::device().destroy_renderer(ch);
TESTFUNC_END

//--------------------------------------------------------------------------------------------------
TESTFUNC_BEGIN(rendereredit_renderconfig)
	tz::gl::RendererInfo rinfo;
	rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
	rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));

	tz::gl::RendererHandle rh = tz::gl::device().create_renderer(rinfo);

	// Try rendering and make sure it doesn't crash.
	tz::gl::device().get_renderer(rh).render();
	{
		tz::gl::Renderer& ren = tz::gl::device().get_renderer(rh);
		ren.edit
		(
			tz::gl::RendererEditBuilder{}
			.render_state
			({
				.wireframe_mode = true
			})
			.build()
		);
	}
	// Try rendering and make sure it doesn't crash.
	tz::gl::device().get_renderer(rh).render();

	tz::gl::device().destroy_renderer(rh);

TESTFUNC_END

//--------------------------------------------------------------------------------------------------
TESTFUNC_BEGIN(renderer_index_buffer)
	tz::gl::RendererInfo rinfo;
	rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
	rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));
	rinfo.add_resource(tz::gl::BufferResource::from_one(0u,
	{
		.flags = {tz::gl::ResourceFlag::IndexBuffer}
	}));

	tz::gl::RendererHandle rh = tz::gl::device().create_renderer(rinfo);
	tz::gl::device().get_renderer(rh).render();

	tz::gl::device().destroy_renderer(rh);
TESTFUNC_END

//--------------------------------------------------------------------------------------------------
TESTFUNC_BEGIN(renderer_indirect_buffer)
	tz::gl::RendererInfo rinfo;
	rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
	rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));
	rinfo.add_resource(tz::gl::BufferResource::from_one(tz::gl::DrawIndirectCommand{},
	{
		.flags = {tz::gl::ResourceFlag::DrawIndirectBuffer}
	}));

	tz::gl::RendererHandle rh = tz::gl::device().create_renderer(rinfo);
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

		renderer_resource_reference_buffer();
		renderer_resource_reference_image();

		rendereredit_bufferresize();
		rendereredit_imageresize();
		rendereredit_resourcewrite_buffer();
		rendereredit_resourcewrite_image();
		rendereredit_computeconfig();
		rendereredit_renderconfig();

		renderer_index_buffer();
		renderer_indirect_buffer();
	}
	tz::terminate();
}
