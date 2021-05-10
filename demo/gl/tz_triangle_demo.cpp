#include "core/tz.hpp"
#include "gl/device.hpp"
#include "gl/render_pass.hpp"
#include "gl/renderer.hpp"

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
        tz::gl::Renderer renderer = device.create_renderer(renderer_builder);
        while(!tz::window().is_close_requested())
        {
            tz::window().update();
        }
    }
    tz::terminate();
}