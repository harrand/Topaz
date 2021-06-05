#ifndef TZSLC_PREPROCESSOR_HPP
#define TZSLC_PREPROCESSOR_HPP
#include "core/containers/enum_field.hpp"
#include <array>
#include <string>

enum class PreprocessorModule
{
    Sampler,

    Begin = Sampler,
    End
};

class PreprocessorModuleField : public tz::EnumField<PreprocessorModule>
{
public:
    static PreprocessorModuleField All()
    {
        PreprocessorModuleField field;
        for(int i = static_cast<int>(PreprocessorModule::Begin); i < static_cast<int>(PreprocessorModule::End); i++)
        {
            field |= static_cast<PreprocessorModule>(i);
        }
        return field;
    }
};

namespace tzslc
{
    constexpr std::array<const char*, static_cast<int>(PreprocessorModule::End)> preprocessor_module_names{{{"sampler"}}};

    bool preprocess(PreprocessorModuleField modules, std::string& shader_source);
    bool preprocess_samplers(std::string& shader_source);
}

#endif // TZSLC_PREPROCESSOR_HPP