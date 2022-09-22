#include "tz/core/tz.hpp"
#include "tz/core/types.hpp"
#include "tz/gl/api/renderer.hpp"
#include "tz/gl/imported_shaders.hpp"
#include "tz/gl/device.hpp"
#include "tz/gl/renderer.hpp"
#include "tz/gl/resource.hpp"

#include ImportedShaderHeader(empty, vertex)
#include ImportedShaderHeader(empty, fragment)
#include ImportedShaderHeader(empty, compute)
#include ImportedShaderHeader(six_writer, compute)

tz::gl::RendererInfo get_empty(bool compute = false)
{
	tz::gl::RendererInfo rinfo;
	if(compute)
	{
		rinfo.shader().set_shader(tz::gl::ShaderStage::Compute, ImportedShaderSource(empty, compute));
	}
	else
	{
		rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
		rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));
	}
	return rinfo;
}

void empty_renderer(tz::gl::Device& dev)
{
	tz::window().begin_frame();
	tz::gl::Renderer empty = dev.create_renderer(get_empty());
	empty.render(1);
	tz::window().end_frame();
}

void empty_renderer_compute(tz::gl::Device& dev)
{
	tz::gl::Renderer empty = dev.create_renderer(get_empty(true));
	empty.render();
}

void renderer_creation(tz::gl::Device& dev)
{
	tz::window().begin_frame();
	tz::gl::BufferResource bres0 = tz::gl::BufferResource::from_one(5.0f);

	tz::gl::RendererInfo rinfo1 = get_empty();
	tz::gl::Renderer renderer1 = dev.create_renderer(rinfo1);

	rinfo1.add_resource(bres0);
	tz::gl::Renderer renderer2 = dev.create_renderer(rinfo1);

	renderer1.render();
	renderer2.render();
	tz::window().end_frame();
}

void renderer_creation_index_buffer(tz::gl::Device& dev)
{
	tz::window().begin_frame();
	tz::gl::BufferResource bres0 = tz::gl::BufferResource::from_one(5.0f);
	tz::gl::BufferResource ibuf = tz::gl::BufferResource::from_one(0u, tz::gl::ResourceAccess::StaticFixed, {tz::gl::ResourceFlag::IndexBuffer});

	tz::gl::RendererInfo rinfo1 = get_empty();
	tz::gl::Renderer renderer1 = dev.create_renderer(rinfo1);

	rinfo1.add_resource(bres0);
	rinfo1.add_resource(ibuf);
	tz::gl::Renderer renderer2 = dev.create_renderer(rinfo1);

	renderer1.render();
	renderer2.render();
	tz::window().end_frame();
}

void renderer_edit(tz::gl::Device& dev)
{
	tz::gl::BufferResource bres0 = tz::gl::BufferResource::from_many({5.0f, 6.0f}, tz::gl::ResourceAccess::DynamicVariable);

	tz::gl::RendererInfo rinfo1 = get_empty();
	tz::gl::ResourceHandle bh = rinfo1.add_resource(bres0);

	tz::gl::Renderer ren = dev.create_renderer(rinfo1);
	tz_assert(bres0.data().size_bytes() == sizeof(float) * 2, "BufferResource had unexpected size before Renderer edit. Expected %zu, got %zu", sizeof(float) * 2, bres0.data().size_bytes());
	// Try to resize it larger, and then smaller. Both should work.
	ren.edit(tz::gl::RendererEditBuilder{}.buffer_resize
	({
		.buffer_handle = bh,
		.size = sizeof(float) * 3
	}).build());
	
	// Ensure data was preserved (first 2 floats).
	{
		auto data = ren.get_resource(bh)->data_as<float>();
		tz_assert(data.size() == 3, "After a buffer resource was resized via Renderer edit, the buffer had unexpected size. Expected %u, but got %zu", 3, data.size());
		tz_assert(data[0] == 5.0f && data[1] == 6.0f, "After a buffer resource was resized via Renderer edit, the data was not preserved as required (buffer grew in size). Expected {%g, %g} but got {%g, %g}", data[0], data[1], 5.0f, 6.0f);
	}

	ren.edit(tz::gl::RendererEditBuilder{}.buffer_resize
	({
		.buffer_handle = bh,
		.size = sizeof(float) * 1
	}).build());

	// Now that the buffer is only 1 float in size, that single float better still be correct.
	// Ensure data was preserved (first 2 floats).
	{
		auto data = ren.get_resource(bh)->data_as<float>();
		tz_assert(data.size() == 1, "After a buffer resource was resized via Renderer edit, the buffer had unexpected size. Expected %u, but got %zu", 1, data.size());
		tz_assert(data[0] == 5.0f, "After a buffer resource was resized via Renderer edit, the data was not preserved as required (buffer grew in size). Expected {%g} but got {%g}", data[0], 5.0f);
	}
}

void renderer_edit_resource_writes()
{
	// One of each type of resource.
	tz::gl::ResourceHandle
		bsf = tz::nullhand, bdf = tz::nullhand, bdv = tz::nullhand,
		isf = tz::nullhand, idf = tz::nullhand, idv = tz::nullhand;

	struct D
	{
		float x;
		char y;
		int z;
	};

	constexpr std::uint32_t black_pixel[] = {0x000000FF};
	std::span<const std::uint32_t> black_pixel_span{black_pixel, 1};
	constexpr std::uint32_t white_pixel[] = {0xFFFFFFFF};
	std::span<const std::uint32_t> white_pixel_span{white_pixel, 1};

	std::array<tz::gl::BufferResource, 3> bufs
	{
		tz::gl::BufferResource::from_one(D{}),
		tz::gl::BufferResource::from_one(D{}, tz::gl::ResourceAccess::DynamicFixed),
		tz::gl::BufferResource::from_one(D{}, tz::gl::ResourceAccess::DynamicVariable)
	};

	std::array<tz::gl::ImageResource, 3> imgs
	{
		tz::gl::ImageResource::from_memory(tz::gl::ImageFormat::RGBA32, {1u, 1u}, black_pixel_span),
		tz::gl::ImageResource::from_memory(tz::gl::ImageFormat::RGBA32, {1u, 1u}, black_pixel_span, tz::gl::ResourceAccess::DynamicFixed),
		tz::gl::ImageResource::from_memory(tz::gl::ImageFormat::RGBA32, {1u, 1u}, black_pixel_span, tz::gl::ResourceAccess::DynamicVariable),
	};

	tz::gl::RendererInfo rinfo;
	rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
	rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));
	bsf = rinfo.add_resource(bufs[0]);
	bdf = rinfo.add_resource(bufs[1]);
	bdv = rinfo.add_resource(bufs[2]);
	isf = rinfo.add_resource(imgs[0]);
	idf = rinfo.add_resource(imgs[0]);
	idv = rinfo.add_resource(imgs[0]);

	tz::gl::Renderer renderer = tz::gl::device().create_renderer(rinfo);
	auto do_edit = [&renderer, &white_pixel_span](tz::gl::ResourceHandle h)
	{
		renderer.edit(tz::gl::RendererEditBuilder{}
			.write
			({
				.resource = h,
				.data = std::as_bytes(white_pixel_span)
			})
			.build()
		);
	};
	// Buffer ResourceWrite.
	// StaticFixed
	do_edit(bsf);
	// DynamicFixed
	do_edit(bdf);
	// DynamicVariable
	do_edit(bdv);

	// Image ResourceWrite.
	// StaticFixed
	do_edit(isf);
	// DynamicFixed
	do_edit(idf);
	// DynamicVariable
	do_edit(idv);
}

void resize_window(tz::gl::Device& dev)
{

	tz::gl::Renderer empty = dev.create_renderer(get_empty());
	empty.render();
	tz::window().set_width(tz::window().get_width() + 10.0f);
	empty.render();
}

void wireframe_toggle(tz::gl::Device& dev)
{
	tz::gl::Renderer empty = dev.create_renderer(get_empty());
	empty.render();
	tz::gl::RendererEditBuilder edit;
	edit.render_state
	({
		.wireframe_mode = true
	});
	empty.edit(edit.build());
	empty.render();
}

void renderer_compute_test(tz::gl::Device& dev)
{
	tz::gl::BufferResource number = tz::gl::BufferResource::from_one(1.0f, tz::gl::ResourceAccess::DynamicFixed);

	tz::gl::RendererInfo rinfo;
	rinfo.shader().set_shader(tz::gl::ShaderStage::Compute, ImportedShaderSource(six_writer, compute));
	rinfo.set_options({tz::gl::RendererOption::RenderWait});
	rinfo.set_compute_kernel({1u, 1u, 1u});
	tz::gl::ResourceHandle numbuf = rinfo.add_resource(number);
	tz::gl::Renderer compute = dev.create_renderer(rinfo);

	compute.render();

	float& num = compute.get_resource(numbuf)->data_as<float>().front();
	tz_assert(num == 6.0f, "Compute shader was meant to write 6.0f to num that was initially 0.0f. The current value is %.1ff", num);
	compute.render();
	tz_assert(num == 36.0f, "Compute shader was meant to write 36.0f to num that was initially 6.0f. The current value is %.1ff", num);
}

void resource_references_compute_test(tz::gl::Device& dev)
{
	tz::gl::BufferResource number = tz::gl::BufferResource::from_one(1.0f, tz::gl::ResourceAccess::DynamicFixed);

	tz::gl::RendererInfo rinfo;
	rinfo.shader().set_shader(tz::gl::ShaderStage::Compute, ImportedShaderSource(six_writer, compute));
	rinfo.set_options({tz::gl::RendererOption::RenderWait});
	rinfo.set_compute_kernel({1u, 1u, 1u});
	tz::gl::ResourceHandle numbuf = rinfo.add_resource(number);
	tz::gl::Renderer compute = dev.create_renderer(rinfo);

	tz::gl::RendererInfo rinfo2 = get_empty();
	tz::gl::ResourceHandle refh = rinfo2.add_component(*compute.get_component(numbuf));
	tz::gl::Renderer renderer = dev.create_renderer(rinfo2);

	tz_assert(renderer.get_resource(refh) != nullptr, "Renderer resource reference returned nullptr from a valid handle. Resource references are broken.");
	tz_assert(renderer.get_resource(refh) == compute.get_resource(numbuf), "Renderer resource reference is not exactly equal to the original resource from another renderer. %p != %p", renderer.get_resource(refh), compute.get_resource(numbuf));

	compute.render();
	renderer.render(1);

	float& num = compute.get_resource(numbuf)->data_as<float>().front();
	tz_assert(num == 6.0f, "Compute shader was meant to write 6.0f to num that was initially 0.0f. The current value is %.1ff", num);
	compute.render();
	tz_assert(num == 36.0f, "Compute shader was meant to write 36.0f to num that was initially 6.0f. The current value is %.1ff", num);
}

void semantics(tz::gl::Device& dev)
{
	static_assert(!tz::copyable<tz::gl::Renderer>);
	static_assert(tz::moveable<tz::gl::Renderer>);
	tz::gl::RendererInfo rinfo = get_empty();
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
		.flags = {tz::ApplicationFlag::HiddenWindow}
	});
	{
		tz::gl::Device& dev = tz::gl::device();
		empty_renderer(dev);
		empty_renderer_compute(dev);
		renderer_creation(dev);
		renderer_creation_index_buffer(dev);
		renderer_edit(dev);
		renderer_edit_resource_writes();
		resize_window(dev);
		wireframe_toggle(dev);
		renderer_compute_test(dev);
		resource_references_compute_test(dev);
		semantics(dev);
	}
	tz::terminate();
}
