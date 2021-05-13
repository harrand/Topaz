#include "core/tz.hpp"
#include "core/vector.hpp"
#include "gl/device.hpp"
#include "gl/render_pass.hpp"
#include "gl/renderer.hpp"

struct Vertex
{
    tz::Vec2 pos;
    tz::Vec3 col;
};

tz::gl::RendererElementFormat vertex_format()
{
    tz::BasicList<tz::gl::RendererAttributeFormat> attributes;
    attributes.add(
        {
            .element_attribute_offset = offsetof(Vertex, pos),
            .type = tz::gl::RendererComponentType::Float32x2
        });
    attributes.add(
        {
            .element_attribute_offset = offsetof(Vertex, col),
            .type = tz::gl::RendererComponentType::Float32x3
        });
    return
    {
        .binding_size = sizeof(Vertex),
        .basis = tz::gl::RendererInputFrequency::PerVertexBasis,
        .binding_attributes = attributes
    };
}

int main()
{
    constexpr tz::GameInfo tz_triangle_demo{"tz_triangle_demo", tz::EngineInfo::Version{1, 0, 0}, tz::info()};
    tz::initialise(tz_triangle_demo);
    {
        tz::gl::DeviceBuilder device_builder;
        tz::gl::Device device{device_builder};

        tz::gl::RenderPassBuilder pass_builder;
        pass_builder.add_pass(tz::gl::RenderPassAttachment::ColourDepth);

        tz::gl::RenderPass render_pass = device.create_render_pass(pass_builder);

        tz::gl::RendererBuilder renderer_builder;
        renderer_builder.set_element_format(vertex_format());
        renderer_builder.set_render_pass(render_pass);
        tz::gl::Renderer renderer = device.create_renderer(renderer_builder);
        while(!tz::window().is_close_requested())
        {
            tz::window().update();
        }
    }
    tz::terminate();
}