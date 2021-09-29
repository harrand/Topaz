#include "core/tz.hpp"
#include "gl/device.hpp"
#include "gl/renderer.hpp"
#include "gl/resource.hpp"
#include "gl/output.hpp"
#include "gl/impl/frontend/common/render_pass_info.hpp"

int main()
{
    tz::initialise({"tz_render_pass_test", tz::Version{1, 0, 0}, tz::info()}, tz::ApplicationType::Headless);
    {
        tz::gl::Device device{tz::gl::DeviceBuilder{}};

        // We will need a dummy shader.
        tz::gl::ShaderBuilder shad_builder;
        shad_builder.set_shader_file(tz::gl::ShaderType::VertexShader, ".\\test\\gl\\shader_test.vertex.tzsl");
        shad_builder.set_shader_file(tz::gl::ShaderType::FragmentShader, ".\\test\\gl\\shader_test.fragment.tzsl");
        tz::gl::Shader shader = device.create_shader(shad_builder);

        // Use a render pass builder to create the render pass info struct
        tz::gl::RenderPassBuilder builder;

        tz_assert(!builder.valid(), "Default RenderPassBuilder is wrongly considered valid");
        tz_assert(builder.get_output_format() == tz::gl::TextureFormat::Null, "Default RenderPassBuilder did not have TextureFormat::Null as its output format");

        builder.add_subpass(tz::gl::RenderSubpassInfo{.colour_attachments = {tz::gl::TextureFormat::Rgba32Unsigned}, .depth_attachment = tz::gl::TextureFormat::DepthFloat32});
        
        tz::gl::RenderPassInfo pass_info = builder.get_info();
        // Once we have the info, verify it makes sense
        tz_assert(builder.valid(), "RenderPassBuilder wrongly considered invalid");
        tz_assert(pass_info.subpasses.colour_attachments.length() == 1, "RenderPassInfo's one subpass had unexpected number of colour attachments. Expected 1, got %zu", pass_info.subpasses.colour_attachments.length());
        tz_assert(pass_info.subpasses.colour_attachments[0] == tz::gl::TextureFormat::Rgba32Unsigned, "RenderPassInfo's one subpass had the wrong colour attachment");
        tz_assert(builder.get_output_format() == tz::gl::TextureFormat::Rgba32Unsigned, "RenderPassBuilder had unexpected output format");
        tz_assert(pass_info.subpasses.depth_attachment.has_value(), "RenderPassInfo's one subpass wrongly says it has no depth attachment");
        tz_assert(pass_info.subpasses.depth_attachment.value() == tz::gl::TextureFormat::DepthFloat32, "RenderPassInfo's one subpass has the wrong depth attachment format");

        // Create an example RendererBuilder and see if we can describe its renderpass correctly
        tz::gl::RendererBuilder ren1;
        ren1.set_output(tz::window());
        // ren1 expects a single subpass, with 1 colour attachment matching the window format and no depth attachment

        tz::gl::RenderPassInfo ren1pass = tz::gl::detail::describe_renderer(ren1, device);
        tz_assert(ren1pass.subpasses.colour_attachments.length() == 1 && ren1pass.subpasses.colour_attachments[0] == device.get_window_format(), "Ren1Pass had unexpected colour attachments in its single subpass");
        tz_assert(ren1pass.subpasses.depth_attachment != std::nullopt, "Ren1Pass wrongly did not have a depth attachment");

        tz::gl::RendererBuilder ren2;
        ren2.set_output(tz::window());
        auto ren2tex_fmt = tz::gl::TextureFormat::Rgba32Signed;
        tz::gl::TextureResource ren2tex{tz::gl::TextureData::uninitialised(1, 1, ren2tex_fmt), ren2tex_fmt};
        tz::gl::ResourceHandle ren2res = ren2.add_resource(ren2tex);
        ren2.set_shader(shader);
        // ren2 expects a single subpass, with 1 colour attachment matching the window format and a depth attachment of DepthFloat32

        tz::gl::RenderPassInfo ren2pass = tz::gl::detail::describe_renderer(ren2, device);
        tz_assert(ren2pass.subpasses.colour_attachments.length() == 1 && ren2pass.subpasses.colour_attachments[0] == device.get_window_format(), "Ren2Pass had unexpected colour attachments in its single subpass");
        tz_assert(ren2pass.subpasses.depth_attachment == tz::gl::TextureFormat::DepthFloat32, "Ren2Pass had unexpected depth attachment for its single subpass");

        tz::gl::Renderer ren2renderer = device.create_renderer(ren2);

        tz::gl::RendererBuilder ren3;
        tz::gl::TextureOutput ren2_output;
        ren2_output.add_colour_output(static_cast<tz::gl::TextureComponent*>(ren2renderer.get_component(ren2res)));
        ren3.set_output(ren2_output);
        // ren3 expects a single subpass, with 1 colour attachment matching ren2tex_fmt and a depth attachment of DepthFloat32

        tz::gl::RenderPassInfo ren3pass = tz::gl::detail::describe_renderer(ren3, device);
        tz_assert(ren3pass.subpasses.colour_attachments.length() == 1 && ren3pass.subpasses.colour_attachments[0] == ren2tex_fmt, "Ren3Pass had unexpected colour attachments in its single subpass");
        tz_assert(ren3pass.subpasses.depth_attachment == tz::gl::TextureFormat::DepthFloat32, "Ren3Pass had unexpected depth attachment for its single subpass");
    }
    tz::terminate();
}