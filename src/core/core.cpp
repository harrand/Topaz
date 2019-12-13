//
// Created by Harrand on 13/12/2019.
//

#include "core/core.hpp"
#include "core/debug/assert.hpp"
#include "core/debug/print.hpp"
// This will initialise *all* modules.
#include "core/tz_glfw/glfw_context.hpp"

namespace tz::core
{
    TopazCore::TopazCore() noexcept: initialised(false) {}

    void TopazCore::initialise(const char* app_name)
    {
        topaz_assert(!this->initialised, "TopazCore::initialise(): Attempt to initialise but we're already marked as initialised!");
        this->initialised = true;
        // TODO: Do initialise stuff here.
        tz::ext::glfw::initialise(tz::ext::glfw::WindowCreationArgs{app_name, 1920, 1080});

        tz::debug_printf("tz::initialise(): Success\n");
    }

    void TopazCore::terminate()
    {
        topaz_assert(this->initialised, "TopazCore::terminate(): Attempt to terminate but we're not marked as initialised!");
        this->initialised = false;
        // TODO: Burn stuff here.
        tz::ext::glfw::terminate();

        tz::debug_printf("tz::terminate(): Success\n");
    }

    bool TopazCore::is_initialised() const
    {
        return this->initialised;
    }

    const tz::ext::glfw::GLFWContext& TopazCore::get_context() const
    {
        return tz::ext::glfw::get();
    }

    static TopazCore global_core;

    void initialise(const char* app_name)
    {
        global_core.initialise(app_name);
    }

    void terminate()
    {
        global_core.terminate();
    }

    const TopazCore& get()
    {
        topaz_assert(global_core.is_initialised(), "Attempted tz::core::get() but tz::initialise() has not yet been invoked!");
        return global_core;
    }
}

