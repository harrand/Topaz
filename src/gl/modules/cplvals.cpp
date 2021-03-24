#include "gl/modules/cplvals.hpp"

namespace tz::gl::p
{
    CompileTimeValueModule::CompileTimeValueModule(): IModule()
    {

    }

    void CompileTimeValueModule::operator()(std::string& source) const
    {
        src::transform(source, std::regex{"#@([a-zA-Z]+)"}, [this](auto beg, auto end)
        {
            topaz_assert(std::distance(beg, end) == 1, "tz::gl::p::CompileTimeValueModule::operator(): Had unexpected number of inner matches. Expected ", 1, ", got ", std::distance(beg, end));
            std::string cplval_name = *beg;

            return this->get_cplval(cplval_name);
        });
    }

    void CompileTimeValueModule::set(std::string name, std::string value)
    {
        this->cplval_name_to_val[name] = value;
    }

    const std::string& CompileTimeValueModule::get_cplval(const std::string& name) const
    {
        topaz_assertf(this->cplval_name_to_val.find(name) != this->cplval_name_to_val.end(), "tz::gl::p::CompileTimeValueModule::get_cplval(\"%s\"): The cplval \"%s\" has not had a value set before preprocessing.", name.c_str(), name.c_str());
        return this->cplval_name_to_val.at(name);
    }
}