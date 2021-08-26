#include "core/assert.hpp"
#include "preprocessor.hpp"
#include "source_transform.hpp"

namespace tzslc
{
    constexpr const char* tzslc_platform_defines()
    {
        #if TZ_VULKAN
            return "#define TZ_OGL 0\n#define TZ_VULKAN 1\n";
        #elif TZ_OGL
            return "#define TZ_OGL 1\n#define TZ_VULKAN 0\n";
        #else
            #error "Neither Vulkan nor OGL render-api backend specified";
        #endif
    }

    void add_glsl_header_info(std::string& shader_source)
    {
        std::string hdrinfo = "/*tzslc header info begin*/\n#version 450 core\n";
        hdrinfo += tzslc_platform_defines();
        hdrinfo += "/*tzslc header info end*/\n";
        shader_source = hdrinfo + shader_source;
    }

    bool preprocess(PreprocessorModuleField modules, std::string& shader_source, std::string& meta)
    {
        bool done_any_work = false;
        add_glsl_header_info(shader_source);
        preprocess_topaz_types(shader_source, meta);
        if(modules.contains(PreprocessorModule::Sampler))
        {
            done_any_work |= preprocess_samplers(shader_source, meta);
        }
        return done_any_work;
    }

    bool preprocess_samplers(std::string& shader_source, std::string& meta)
    {
       tzslc::transform(shader_source, std::regex{"resource\\(id ?= ?([0-9]+)\\) const texture"}, [&](auto beg, auto end)->std::string
       {
            tz_assert(std::distance(beg, end) == 1, "resource(id = x) const texture <name> : 'x' should be one number");
            int id = std::stoi(*beg);
            std::string replacement = "/*tzslc: const texture*/ layout(";
            #if TZ_VULKAN
                replacement += "binding";
            #elif TZ_OGL
                replacement += "location";
            #endif
            replacement += " = " + std::to_string(id) + ") uniform sampler2D";
            meta += std::to_string(id) + " = texture\n";
            return replacement;
       });
       return false;
    }

    bool preprocess_topaz_types(std::string& shader_source, std::string& meta)
    {
        /*
        example tzsl:
        resource(id = 0) const buffer MVP
        {
            mat4 model;
            mat4 view;
            mat4 projection;
        } mvp;

        desired output (vulkan & opengl):
        layout(binding = 0) uniform MVP
        {
            mat4 model;
            mat4 view;
            mat4 projection;
        } mvp;
        */
       // Handle 'const buffer' (UBO)
       tzslc::transform(shader_source, std::regex{"resource\\(id ?= ?([0-9]+)\\) const buffer"}, [&meta](auto beg, auto end)->std::string
       {
            tz_assert(std::distance(beg, end) == 1, "resource(id = x) const buffer <name> : 'x' should be one number");
            int id = std::stoi(*beg);
            std::string replacement = "/*tzslc: const buffer resource (ubo)*/ layout(binding = ";
            replacement += std::to_string(id);
            replacement += ") uniform";

            meta += std::to_string(id) + " = ubo\n";
            return replacement;
       });
       // Handle 'buffer' (SSBO)
       tzslc::transform(shader_source, std::regex{"resource\\(id ?= ?([0-9]+)\\) buffer"}, [&meta](auto beg, auto end)->std::string
       {
            tz_assert(std::distance(beg, end) == 1, "resource(id = x) buffer : 'x' should be one number");
            int id = std::stoi(*beg);
            std::string replacement = "/*tzslc buffer resource (ssbo)*/ layout(binding = ";
            replacement += std::to_string(id);
            replacement += ") buffer";
            meta += std::to_string(id) + " = ssbo\n";
            return replacement;
       });
       return false;
    }
}