#include "gl/modules/ssbo.hpp"
#include "gl/object.hpp"
#include <regex>
#include <sstream>

namespace tz::gl::p
{
    SSBOModule::SSBOModule(tz::gl::Object* o): ObjectAwareModule(o){}

    void SSBOModule::operator()(std::string& source) const
    {
        using SSBOJob = std::pair<std::pair<std::size_t, std::size_t>, std::string>;
        std::vector<SSBOJob> ssbo_processing_list;
        // Firstly, find all ssbos in the source. We need to keep track of each ssbo name.
        std::string src_copy = source;
        std::regex reg{"#ssbo (.+)"};
        std::smatch sm;
        std::ptrdiff_t src_pos_counter = 0;
        while(std::regex_search(src_copy, sm, reg))
        {
            // Found an ssbo!
            std::ptrdiff_t pos = sm.position() + src_pos_counter;
            topaz_assert(pos < source.size(), "tz::gl::p::SSBOModule::operator(...): Match result has position ", pos, " in source which is out of range. Size: ", source.size());
            std::ptrdiff_t len = sm.length();
            std::string ssbo_name = sm[1];
            src_copy = sm.suffix();
            src_pos_counter += pos + len;
            // Register an SSBO.
            std::pair<std::size_t, std::size_t> pos_and_length{pos, len};
            // Create an SSBO in the object and let's get its ID.
            std::size_t ssbo_id = this->o->emplace_buffer<tz::gl::BufferType::ShaderStorage>(this->o->size());
            tz::gl::SSBO* ssbo = this->o->get<tz::gl::BufferType::ShaderStorage>(ssbo_id);
            this->ssbo_name_id.emplace_back(ssbo_name, ssbo_id);
            std::stringstream ss;
            ss << "layout(std430, binding = ";
            ss << ssbo->get_binding_id();
            ss << ") buffer ";
            ss << ssbo_name;
            SSBOJob job{pos_and_length, ss.str()};
            ssbo_processing_list.push_back(job);
        }

        // Now invoke all jobs in reverse-order.
		for(auto i = ssbo_processing_list.rbegin(); i != ssbo_processing_list.rend(); i++)
		{
			std::string replacement = i->second;
			std::size_t pos = i->first.first;
			std::size_t len = i->first.second;
			source.replace(pos, len, replacement);
		}
    }

    std::size_t SSBOModule::size() const
    {
        return this->ssbo_name_id.size();
    }

    const std::string& SSBOModule::get_name(std::size_t idx) const
    {
        return this->ssbo_name_id[idx].first;
    }

    std::size_t SSBOModule::get_buffer_id(std::size_t idx) const
    {
        return this->ssbo_name_id[idx].second;
    }
}