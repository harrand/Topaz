#include "core/tz.hpp"
#include "gl/device.hpp"
#include "gl/renderer.hpp"
#include "gl/resource.hpp"
#include "gl/impl/frontend/common/render_pass.hpp"

int main()
{
    tz::initialise({"tz_render_pass_test", tz::Version{1, 0, 0}, tz::info()}, tz::ApplicationType::Headless);
    {
        tz::gl::RenderPassBuilder builder;

        tz_assert(!builder.valid(), "Default RenderPassBuilder is wrongly considered valid");
        tz_assert(builder.get_output_format() == tz::gl::TextureFormat::Null, "Default RenderPassBuilder did not have TextureFormat::Null as its output format");

        builder.set_output_format(tz::gl::TextureFormat::Rgba32Signed);
        tz_assert(builder.get_output_format() == tz::gl::TextureFormat::Rgba32Signed, "RenderPassBuilder::set_output_format (or associated get) is not functioning correctly");
    }
    tz::terminate();
}