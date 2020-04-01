#include "gl/modules/bindless_sampler.hpp"

namespace tz::gl::p
{
    void BindlessSamplerModule::operator()(std::string& source) const
    {
        src::transform(source, std::regex{"tz_bindless_sampler"}, [this]([[maybe_unused]] auto beg, [[maybe_unused]] auto end)
		{
	        return "sampler2D";
		});
    }
}