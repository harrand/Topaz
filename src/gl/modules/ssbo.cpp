#include "gl/modules/ssbo.hpp"
#include "gl/object.hpp"
#include <regex>
#include <sstream>

namespace tz::gl::p
{
    SSBOModule::SSBOModule(tz::gl::Object* o): ObjectAwareModule(o){}

    void SSBOModule::operator()(std::string& source) const
    {
        src::transform(source, std::regex{"#ssbo (.+)"}, [this](auto beg, auto end)
        {
            topaz_assert(std::distance(beg, end) == 1, "tz::gl::p::SSBOModule::operator(): Had unexpected number of inner matches. Expected ", 1, ", got ", std::distance(beg, end));
            std::string ssbo_name = *beg;

            std::size_t ssbo_id = this->o->emplace_buffer<tz::gl::BufferType::ShaderStorage>(this->o->size());
            tz::gl::SSBO* ssbo = this->o->get<tz::gl::BufferType::ShaderStorage>(ssbo_id);
            this->ssbo_name_id.emplace_back(ssbo_name, ssbo_id);

            std::stringstream ss;
            ss << "layout(std430, binding = ";
            ss << ssbo->get_binding_id();
            ss << ") buffer ";
            ss << ssbo_name;
            return ss.str();
        });
    }

    std::size_t SSBOModule::size() const
    {
        return this->ssbo_name_id.size();
    }

    const std::string& SSBOModule::get_name(std::size_t idx) const
    {
        topaz_assert(idx < this->size(), "tz::gl::p::SSBOModule::get_name(", idx, "): Index ", idx, " is out of range! Size: ", this->size());
        return this->ssbo_name_id[idx].first;
    }

    std::size_t SSBOModule::get_buffer_id(std::size_t idx) const
    {
        topaz_assert(idx < this->size(), "tz::gl::p::SSBOModule::get_buffer_id(", idx, "): Index ", idx, " is out of range! Size: ", this->size());
        return this->ssbo_name_id[idx].second;
    }
}