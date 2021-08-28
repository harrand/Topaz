#include "gl/impl/frontend/common/shader.hpp"
#include "core/types.hpp"
#include "core/assert.hpp"
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
}