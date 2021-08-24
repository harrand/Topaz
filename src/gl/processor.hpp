#ifndef TOPAZ_GL_PROCESSOR_HPP
#define TOPAZ_GL_PROCESSOR_HPP

#if TZ_VULKAN
#include "gl/impl/frontend/vk/processor.hpp"
namespace tz::gl
{
    using ProcessorBuilder = ProcessorBuilderVulkan;
    using Processor = ProcessorVulkan;
}
#elif TZ_OGL
//#include "gl/impl/frontend/ogl/processor.hpp"
namespace tz::gl
{
    //using ProcessorBuilder = ProcessorBuilderOGL;
    //using Processor = ProcessorOGL;
    class ProcessorBuilder{};
    class Processor{public: Processor([[maybe_unused]] ProcessorBuilder builder){}};
}
#endif

#endif // TOPAZ_GL_RENDERER_HPP