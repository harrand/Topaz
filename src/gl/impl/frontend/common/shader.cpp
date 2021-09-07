#include "gl/impl/frontend/common/shader.hpp"
#include "core/types.hpp"
#include "core/assert.hpp"
#include <fstream>
#include <sstream>
#include <regex>
#include <utility>

namespace tz::gl
{
    namespace detail
    {
        using SourceMatchIteratorType = std::vector<std::string>::iterator;
    }
    template<typename F>
    concept SourceFindAction = tz::Action<F, detail::SourceMatchIteratorType, detail::SourceMatchIteratorType>;
    void search(std::string& source, std::regex reg, SourceFindAction auto search_function)
    {        
        std::string src_copy = source;
        std::smatch sm;
        std::size_t src_pos_counter = 0;

        while(std::regex_search(src_copy, sm, reg))
        {
            // Found a match!
            std::size_t pos = sm.position() + src_pos_counter;
            tz_assert(pos < source.size(), "Match result has position %zu in source which is out of range. Size: %zu", pos, source.size());
            std::vector<std::string> inner_matches;
            for(std::size_t i = 1; i < sm.size(); i++)
                inner_matches.push_back(sm[i]);
            src_copy = sm.suffix();
            src_pos_counter = source.find(src_copy);
            //src_pos_counter += pos + len;
            // get the transformed string.
            search_function(inner_matches.begin(), inner_matches.end());
        }
    }

    ShaderMeta ShaderMeta::from_metadata_string(const std::string& metadata)
    {
        ShaderMeta meta;
        
        std::istringstream f(metadata.c_str());
        std::string line;
        while(std::getline(f, line))
        {
            search(line, std::regex{"([0-9]+) = (.+)"}, [&meta](auto beg, auto end)
            {
                tz_assert(std::distance(beg, end) == 2, "Regex fail");
                int id = std::stoi(*beg);
                std::advance(beg, 1);
                std::string type = *beg;
                // Try to convert to an actual type.
                ShaderMetaValue val = ShaderMetaValue::Count;
                const char* type_cstr = type.c_str();
                {
                    for(std::size_t i = 0; std::cmp_less(i, static_cast<int>(ShaderMetaValue::Count)); i ++)
                    {
                        const char* meta_value_name = detail::meta_value_names[i];
                        if(std::strcmp(type_cstr, meta_value_name) == 0)
                        {
                            val = static_cast<ShaderMetaValue>(i);
                            break;
                        }
                    }
                }
                tz_assert(val != ShaderMetaValue::Count, "Failed to recognise meta value type with name \"%s\"", type_cstr);
                meta.resource_types[id] = val;
            });
        }
        return meta;
    }

    std::optional<ShaderMetaValue> ShaderMeta::try_get_meta_value(unsigned int resource_id) const
    {
        if(this->resource_types.contains(resource_id))
        {
            return this->resource_types.at(resource_id);
        }
        else
        {
            return std::nullopt;
        }
    }

    void ShaderBuilderBase::set_shader_file(ShaderType type, std::filesystem::path shader_file)
    {
        auto read_all = [](std::filesystem::path path)->std::string
        {
            std::ifstream fstr{path.c_str(), std::ios::ate | std::ios::binary};
            if(!fstr.is_open())
            {
                return "";
            }
            auto file_size_bytes = static_cast<std::size_t>(fstr.tellg());
            fstr.seekg(0);
            std::string buffer;
            buffer.resize(file_size_bytes);
            fstr.read(buffer.data(), file_size_bytes);
            fstr.close();
            return buffer;
        };
        std::filesystem::path compiled_file = shader_file;
        #if TZ_VULKAN
            compiled_file += ".spv";
        #elif TZ_OGL
            compiled_file += ".glsl";
        #endif
        this->set_shader_source(type, read_all(compiled_file));

        // Now find the meta file if there is one
        std::filesystem::path meta_file = shader_file;
        meta_file += ".glsl.meta";
        if(std::filesystem::exists(meta_file))
        {
            this->set_shader_meta(type, read_all(meta_file));
        }
    }

    void ShaderBuilderBase::set_shader_source(ShaderType type, std::string source_code)
    {
        switch(type)
        {
            case ShaderType::VertexShader:
                this->vertex.source = source_code;
            break;
            case ShaderType::FragmentShader:
                this->fragment.source = source_code;
            break;
            case ShaderType::ComputeShader:
                this->compute.source = source_code;
            break;
            default:
                tz_error("Shader type (write) is not supported on Vulkan");
            break;
        }
    }

    void ShaderBuilderBase::set_shader_meta(ShaderType type, std::string metadata)
    {
        switch(type)
        {
            case ShaderType::VertexShader:
                this->vertex.metadata = metadata;
            break;
            case ShaderType::FragmentShader:
                this->fragment.metadata = metadata;
            break;
            case ShaderType::ComputeShader:
                this->compute.metadata = metadata;
            break;
            default:
                tz_error("Shader type (read) is not supported on Vulkan");
            break;
        }
    }

    std::string_view ShaderBuilderBase::get_shader_source(ShaderType type) const
    {
        switch(type)
        {
            case ShaderType::VertexShader:
                return this->vertex.source;
            break;
            case ShaderType::FragmentShader:
                return this->fragment.source;
            break;
            case ShaderType::ComputeShader:
                return this->compute.source;
            break;
            default:
                tz_error("Shader type (read) is not supported on Vulkan");
                return "";
            break;
        }
    }

    std::string_view ShaderBuilderBase::get_shader_meta(ShaderType type) const
    {
        switch(type)
        {
            case ShaderType::VertexShader:
                return this->vertex.metadata;
            break;
            case ShaderType::FragmentShader:
                return this->fragment.metadata;
            break;
            case ShaderType::ComputeShader:
                return this->compute.metadata;
            break;
            default:
                tz_error("Shader type (read) is not supported on Vulkan");
                return "";
            break;
        }
    }


    bool ShaderBuilderBase::has_shader(ShaderType type) const
    {
        switch(type)
        {
            case ShaderType::VertexShader:
                return !this->vertex.source.empty();
            break;
            case ShaderType::FragmentShader:
                return !this->fragment.source.empty();
            break;
            case ShaderType::ComputeShader:
                return !this->compute.source.empty();
            break;
            default:
                tz_error("Shader type (exist) is not supported on Vulkan");
                return false;
            break;
        }
    }
}