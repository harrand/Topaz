#include "gl/modules/ubo.hpp"
#include "gl/object.hpp"
#include <regex>
#include <sstream>

namespace tz::gl::p
{
	UBOModule::UBOModule(tz::gl::Object& o): ObjectAwareModule(o){}

	void UBOModule::operator()(std::string& source) const
	{
		src::transform(source, std::regex{"#ubo (.+)"}, [this](auto beg, auto end)
		{
			topaz_assert(std::distance(beg, end) == 1, "tz::gl::p::UBOModule::operator(): Had unexpected number of inner matches. Expected ", 1, ", got ", std::distance(beg, end));
			std::string ubo_name = *beg;

			std::size_t ubo_id;
			std::optional<std::size_t> possible_id = this->get_ubo_id_by_name(ubo_name);
			if(possible_id.has_value())
			{
				// we've seen this ubo name before. use the existing id.
				ubo_id = possible_id.value();
			}
			else
			{
				// haven't seen this before, add it.
				ubo_id = this->o->emplace_buffer<tz::gl::BufferType::UniformStorage>(this->o->size());
				this->ubo_name_id.emplace_back(ubo_name, ubo_id);
			}
			tz::gl::UBO* ubo = this->o->get<tz::gl::BufferType::UniformStorage>(ubo_id);

			std::stringstream ss;
			ss << "layout(std140, binding = ";
			ss << ubo->get_binding_id();
			ss << ") uniform ";
			ss << ubo_name;
			return ss.str();
		});
	}

	std::size_t UBOModule::size() const
	{
		return this->ubo_name_id.size();
	}

	const std::string& UBOModule::get_name(std::size_t idx) const
	{
		return this->ubo_name_id[idx].first;
	}

	std::size_t UBOModule::get_buffer_id(std::size_t idx) const
	{
		return this->ubo_name_id[idx].second;
	}

	std::size_t UBOModule::get_buffer_id(const std::string& ubo_name) const
	{
		auto optional_sizet = this->get_ubo_id_by_name(ubo_name);
		topaz_assertf(optional_sizet.has_value(), "tz::gl::p::UBOModule::get_buffer_id(%s): No such UBO with that name present.", ubo_name.c_str());
		return optional_sizet.value();
	}

	std::optional<std::size_t> UBOModule::get_ubo_id_by_name(const std::string& ubo_name) const
	{
		auto iter = std::find_if(this->ubo_name_id.begin(), this->ubo_name_id.end(), [&ubo_name](auto pair)
		{
			return pair.first == ubo_name;
		});
		if(iter != this->ubo_name_id.end())
		{
			return {iter->second};
		}
		return {std::nullopt};
	}
}