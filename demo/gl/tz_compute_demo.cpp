#include "core/tz.hpp"
#include "core/vector.hpp"
#include "core/matrix_transform.hpp"
#include "gl/device.hpp"
#include "gl/processor.hpp"
#include "gl/resource.hpp"
#include "gl/input.hpp"
#include "gl/shader.hpp"

float get_aspect_ratio()
{
    return tz::window().get_width() / tz::window().get_height();
}

int main()
{
    tz::WindowInitArgs wargs = tz::default_args;
    wargs.resizeable = false;
    tz::initialise({"tz_compute_demo", tz::Version{1, 0, 0}, tz::info()}, tz::ApplicationType::WindowApplication, wargs);
    {
        tz::gl::DeviceBuilder device_builder;
        tz::gl::Device device{device_builder};

        tz::gl::ShaderBuilder shader_builder;
        shader_builder.set_shader_file(tz::gl::ShaderType::ComputeShader, ".\\demo\\gl\\compute_demo.compute.tzsl");

        tz::gl::Shader shader = device.create_shader(shader_builder);

        tz::gl::ProcessorBuilder processor_builder;
        tz::gl::BufferResource buf_res{tz::gl::BufferData::from_array<tz::Mat4>
        ({{
            tz::model({0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}),
            tz::view({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}),
            tz::perspective(1.27f, get_aspect_ratio(), 0.1f, 1000.0f)
        }})};
        processor_builder.add_resource(buf_res);
        processor_builder.set_shader(shader);
        tz::gl::Processor processor = device.create_processor(processor_builder);

        while(!tz::window().is_close_requested())
        {
            tz::window().update();
            processor.process();
        }
    }
    tz::terminate();
}