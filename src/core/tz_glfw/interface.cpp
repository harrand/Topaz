#include "core/tz_glfw/interface.hpp"
#include "core/debug/assert.hpp"
#include "GLFW/glfw3.h"

namespace tz::ext::glfw
{
    Interface* glfw_ptr = nullptr;

    void initialise(WindowCreationArgs args, WindowHintCollection hints)
    {
        [[maybe_unused]] int success = glfwInit();
        topaz_assertf(success == GLFW_TRUE, "tz::ext::glfw::Interface(): glfwInit() returned erroneous %d.", success);
        topaz_assert(glfw_ptr == nullptr, "tz::ext::glfw::initialise(): GLFW already initialised!");
        glfw_ptr = new Interface{args, hints};
    }

    void terminate()
    {
        topaz_assert(glfw_ptr != nullptr, "tz::ext::glfw::terminate(): GLFW is not currently initialised!");
        delete glfw_ptr;
        glfwTerminate();
    }


    Interface& get()
    {
        topaz_assert(glfw_ptr != nullptr, "tz::ext::glfw::get(): GLFW not yet initialised! Run initialise first.");
        return *glfw_ptr;
    }

    void default_error_callback(int error_code, const char* msg)
    {
        topaz_assertf(error_code == GLFW_NO_ERROR, "GLFW Error Code %x. Message: \"%s\"", error_code, msg);
    }

    tz::IWindow& Interface::get_window()
    {
        return this->window.functionality();
    }

    Interface::Interface(WindowCreationArgs args, WindowHintCollection hints): window{args, hints}{}
}