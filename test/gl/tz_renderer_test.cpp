#include "core/tz.hpp"
#include "core/types.hpp"
#include "gl/api/renderer.hpp"
#include "gl/imported_shaders.hpp"
#include "gl/device.hpp"
#include "gl/renderer.hpp"
#include "gl/resource.hpp"

#include ImportedShaderHeader(empty, vertex)
#include ImportedShaderHeader(empty, fragment)
#include ImportedShaderHeader(empty, compute)
#include ImportedShaderHeader(six_writer, compute)

void empty_renderer(tz::gl::Device& dev)
{
	tz::gl::RendererInfo rinfo;
	rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
	rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));
	tz::gl::Renderer empty = dev.create_renderer(rinfo);
	empty.render();
}

void empty_renderer_compute(tz::gl::Device& dev)
{
	tz::gl::RendererInfo rinfo;
	rinfo.shader().set_shader(tz::gl::ShaderStage::Compute, ImportedShaderSource(empty, compute));
	tz::gl::Renderer empty = dev.create_renderer(rinfo);
	empty.render();
}

void renderer_creation(tz::gl::Device& dev)
{
	tz::gl::BufferResource bres0 = tz::gl::BufferResource::from_one(5.0f);

	tz::gl::RendererInfo rinfo1;
	rinfo1.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
	rinfo1.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));
	tz::gl::Renderer renderer1 = dev.create_renderer(rinfo1);

	tz::gl::RendererInfo rinfo2 = rinfo1;
	rinfo2.add_resource(bres0);
	tz::gl::Renderer renderer2 = dev.create_renderer(rinfo2);

	renderer1.render();
	renderer2.render();
}

void renderer_creation_index_buffer(tz::gl::Device& dev)
{
	tz::gl::BufferResource bres0 = tz::gl::BufferResource::from_one(5.0f);
	tz::gl::BufferResource ibuf = tz::gl::BufferResource::from_one(0u, tz::gl::ResourceAccess::StaticFixed, {tz::gl::ResourceFlag::IndexBuffer});

	tz::gl::RendererInfo rinfo1;
	rinfo1.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
	rinfo1.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));
	tz::gl::Renderer renderer1 = dev.create_renderer(rinfo1);

	tz::gl::RendererInfo rinfo2 = rinfo1;
	rinfo2.add_resource(bres0);
	rinfo2.add_resource(ibuf);
	tz::gl::Renderer renderer2 = dev.create_renderer(rinfo2);

	renderer1.render();
	renderer2.render();
}

void renderer_edit(tz::gl::Device& dev)
{
	tz::gl::BufferResource bres0 = tz::gl::BufferResource::from_many({5.0f, 6.0f}, tz::gl::ResourceAccess::DynamicVariable);

	tz::gl::RendererInfo rinfo1;
	rinfo1.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
	rinfo1.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));
	tz::gl::ResourceHandle bh = rinfo1.add_resource(bres0);

	tz::gl::Renderer ren = dev.create_renderer(rinfo1);
	tz_assert(bres0.data().size_bytes() == sizeof(float) * 2, "BufferResource had unexpected size before Renderer edit. Expected %zu, got %zu", sizeof(float) * 2, bres0.data().size_bytes());
	// Try to resize it larger, and then smaller. Both should work.
	tz::gl::RendererBufferComponentEditRequest bres0_large
	{
		.buffer_handle = bh,
		.size = sizeof(float) * 3
	};
	tz::gl::RendererBufferComponentEditRequest bres0_small
	{
		.buffer_handle = bh,
		.size = sizeof(float) * 1
	};
	tz::gl::RendererEditRequest req;
	// Resize large
	req.component_edits = {bres0_large};
	ren.edit(req);
	
	// Ensure data was preserved (first 2 floats).
	{
		auto data = ren.get_resource(bh)->data_as<float>();
		tz_assert(data.size() == 3, "After a buffer resource was resized via Renderer edit, the buffer had unexpected size. Expected %u, but got %zu", 3, data.size());
		tz_assert(data[0] == 5.0f && data[1] == 6.0f, "After a buffer resource was resized via Renderer edit, the data was not preserved as required (buffer grew in size). Expected {%g, %g} but got {%g, %g}", data[0], data[1], 5.0f, 6.0f);
	}

	// Resize small
	req.component_edits = {bres0_small};
	ren.edit(req);

	// Now that the buffer is only 1 float in size, that single float better still be correct.
	// Ensure data was preserved (first 2 floats).
	{
		auto data = ren.get_resource(bh)->data_as<float>();
		tz_assert(data.size() == 1, "After a buffer resource was resized via Renderer edit, the buffer had unexpected size. Expected %u, but got %zu", 1, data.size());
		tz_assert(data[0] == 5.0f, "After a buffer resource was resized via Renderer edit, the data was not preserved as required (buffer grew in size). Expected {%g} but got {%g}", data[0], 5.0f);
	}
}

void resize_window(tz::gl::Device& dev)
{
	tz::gl::RendererInfo rinfo;
	rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
	rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));
	tz::gl::Renderer empty = dev.create_renderer(rinfo);
	empty.render();
	tz::window().set_width(tz::window().get_width() + 10.0f);
	empty.render();
}

void renderer_compute_test(tz::gl::Device& dev)
{
	tz::gl::BufferResource number = tz::gl::BufferResource::from_one(0.0f, tz::gl::ResourceAccess::DynamicFixed);

	tz::gl::RendererInfo rinfo;
	rinfo.shader().set_shader(tz::gl::ShaderStage::Compute, ImportedShaderSource(six_writer, compute));
	tz::gl::ResourceHandle numbuf = rinfo.add_resource(number);
	tz::gl::Renderer compute = dev.create_renderer(rinfo);


	compute.render();

	float num = compute.get_resource(numbuf)->data_as<float>().front();
	tz_assert(num == 6.0f, "Compute shader was meant to write 6.0f to num that was initially 0.0f. The current value is %.1ff", num);
}

void semantics(tz::gl::Device& dev)
{
	static_assert(!tz::copyable<tz::gl::Renderer>);
	static_assert(tz::moveable<tz::gl::Renderer>);
	tz::gl::RendererInfo rinfo;
	rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
	rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));
	tz::gl::Renderer empty = dev.create_renderer(rinfo);
	tz::gl::Renderer empty2 = dev.create_renderer(rinfo);
	tz::gl::Renderer empty_mv = std::move(empty);
	empty_mv.render();
	
	empty2 = std::move(empty_mv);
	empty2.render();
}

int main()
{
	tz::initialise
	({
		.name = "tz_renderer_test",
		.app_type = tz::ApplicationType::HiddenWindowApplication
	});
	{
		tz::gl::Device dev;
		empty_renderer(dev);
		empty_renderer_compute(dev);
		renderer_creation(dev);
		renderer_creation_index_buffer(dev);
		renderer_edit(dev);
		resize_window(dev);
		renderer_compute_test(dev);
		semantics(dev);
	}
	tz::terminate();
}
