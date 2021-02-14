#include "gl/modules/ssbo.hpp"
#include "gl/object.hpp"
#include <regex>
#include <sstream>

namespace tz::gl::p
{
	SSBOModule::SSBOModule(tz::gl::Object& o): ObjectAwareModule(o){}

	void SSBOModule::operator()(std::string& source) const
	{
		src::transform(source, std::regex{"#ssbo (.+)"}, [this](auto beg, auto end)
		{
			topaz_assert(std::distance(beg, end) == 1, "tz::gl::p::SSBOModule::operator(): Had unexpected number of inner matches. Expected ", 1, ", got ", std::distance(beg, end));
			std::string ssbo_name = *beg;

			std::optional<std::size_t> maybe_existing_ssbo_id = this->get_existing_ssbo_id(ssbo_name);
			std::size_t ssbo_id;
			tz::gl::SSBO* ssbo;
			if(maybe_existing_ssbo_id.has_value())
			{
				ssbo_id = maybe_existing_ssbo_id.value();
			}
			else
			{
				ssbo_id = this->o->emplace_buffer<tz::gl::BufferType::ShaderStorage>(this->o->size());
				this->ssbo_name_id.emplace_back(ssbo_name, ssbo_id);
			}
			
			ssbo = this->o->get<tz::gl::BufferType::ShaderStorage>(ssbo_id);
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

	std::optional<std::size_t> SSBOModule::get_existing_ssbo_id(const std::string& ssbo_name) const
	{
		auto iter = std::find_if(this->ssbo_name_id.begin(), this->ssbo_name_id.end(), [&ssbo_name](std::pair<std::string, std::size_t> entry)->bool
		{
			return entry.first == ssbo_name;
		});
		if(iter != this->ssbo_name_id.end())
		{
			return {iter->second};
		}
		return std::nullopt;
	}

}