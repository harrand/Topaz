//
// Created by Harrand on 13/12/2019.
//

#ifndef TOPAZ_CORE_HPP
#define TOPAZ_CORE_HPP

// Forward declare
namespace tz::ext::glfw
{
    class GLFWContext;
}

namespace tz::core
{
    class TopazCore
    {
    public:
        // Ignore ctor/dtor as we expect this to be a global.
        // Also break the standard and define as noexcept.
        TopazCore() noexcept;
        ~TopazCore() = default;
        void initialise(const char* app_name);
        void terminate();

        bool is_initialised() const;
        const tz::ext::glfw::GLFWContext& get_context() const;
    private:
        bool initialised;
    };

    void initialise(const char* app_name);
    void terminate();
    const TopazCore& get();
}

#endif //TOPAZ_CORE_HPP
