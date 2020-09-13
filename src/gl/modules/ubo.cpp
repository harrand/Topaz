#include "gl/modules/ubo.hpp"
#include "gl/object.hpp"
#include <regex>
#include <sstream>

namespace tz::gl::p
{
	UBOModule::UBOModule(tz::gl::Object* o): ObjectAwareModule(o){}

	void UBOModule::operator()(std::string& source) const
	{
		src::transform(source, std::regex{"#ubo (.+)"}, [this](auto beg, auto end)
		{
			topaz_assert(std::distance(beg, end) == 1, "tz::gl::p::UBOModule::operator(): Had unexpected number of inner matches. Expected ", 1, ", got ", std::distance(beg, end));
			std::string ubo_name = *beg;

			std::size_t ubo_id = this->o->emplace_buffer<tz::gl::BufferType::UniformStorage>(this->o->size());
			tz::gl::UBO* ubo = this->o->get<tz::gl::BufferType::UniformStorage>(ubo_id);
			this->ubo_name_id.emplace_back(ubo_name, ubo_id);

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
}