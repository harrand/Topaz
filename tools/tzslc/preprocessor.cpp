#include "core/assert.hpp"
#include "preprocessor.hpp"
#include "source_transform.hpp"

namespace tzslc
{
    bool preprocess(PreprocessorModuleField modules, std::string& shader_source)
    {
        bool done_any_work = false;
        if(modules.contains(PreprocessorModule::Sampler))
        {
            done_any_work |= preprocess_samplers(shader_source);
        }
        return done_any_work;
    }

    bool preprocess_samplers(std::string& shader_source)
    {
        tzslc::transform(shader_source, std::regex{"tz_sampler2D_uniform\\(([0-9]+)\\)"}, [](auto beg, auto end)->std::string
        {
            tz_assert(std::distance(beg, end) == 1, "tz_sampler2D_uniform(x) : 'x' should be one number");
            int id = std::stoi(*beg);
            std::string replacement = "/*tzslc*/ layout(";
            #if TZ_VULKAN
                replacement += "binding";
            #elif TZ_OGL
                replacement += "location";
            #endif
            replacement += " = " + std::to_string(id) + ") uniform sampler2D";
            return replacement;
        });
        return false;
    }
}