#ifndef TOPAZ_GL_MODULE_CPLVALS_HPP
#define TOPAZ_GL_MODULE_CPLVALS_HPP
#include "gl/shader_preprocessor.hpp"
#include <unordered_map>

namespace tz::gl::p
{
    class CompileTimeValueModule : public IModule
    {
    public:
        CompileTimeValueModule();
        virtual void operator()(std::string& source) const override;
        void set(std::string name, std::string value);
    private:
        const std::string& get_cplval(const std::string& name) const;
        std::unordered_map<std::string, std::string> cplval_name_to_val;
    };
}

#endif // TOPAZ_GL_MODULE_CPLVALS_HPP