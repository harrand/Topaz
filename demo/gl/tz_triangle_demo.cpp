#include "core/tz.hpp"
#include "gl/device.hpp"
#include "gl/render_pass.hpp"

int main()
{
    constexpr tz::GameInfo tz_triangle_demo{"tz_triangle_demo", tz::EngineInfo::Version{1, 0, 0}, tz::info()};
    tz::initialise(tz_triangle_demo);
    {
        tz::gl::Device device;

        tz::gl::RenderPassBuilder builder;
        builder.add_pass(tz::gl::RenderPassAttachment::ColourDepth);

        tz::gl::RenderPass render_pass = device.create_render_pass(builder);

        while(!tz::window().is_close_requested())
        {
            tz::window().update();
        }
    }
    tz::terminate();
}