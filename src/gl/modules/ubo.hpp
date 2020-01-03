#ifndef TOPAZ_GL_MODULE_UBO_HPP
#define TOPAZ_GL_MODULE_UBO_HPP
#include "gl/shader_preprocessor.hpp"
#include <vector>
#include <string>

namespace tz::gl::p
{
    class UBOModule : public ObjectAwareModule
    {
    public:
        UBOModule(tz::gl::Object* o);
        virtual void operator()(std::string& source) const override;
        std::size_t size() const;
        const std::string& get_name(std::size_t idx) const;
        std::size_t get_buffer_id(std::size_t idx) const;
    private:
        mutable std::vector<std::pair<std::string, std::size_t>> ubo_name_id;
    };
}

#endif //TOPAZ_GL_MODULE_SSBO_HPP