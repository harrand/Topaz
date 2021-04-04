#ifndef TOPAZ_CORE_TZ_GLFW_INTERFACE_HPP
#define TOPAZ_CORE_TZ_GLFW_INTERFACE_HPP
#include "core/types.hpp"
#include "core/tz_glfw/window_impl.hpp"

namespace tz::ext::glfw
{
    class Interface;
    /*
     * Friend of Interface.
     */
    void initialise(WindowCreationArgs args, WindowHintCollection hints);
    /*
     * Friend of Interface.
     */
    void terminate();
    Interface& get();

    template<typename F>
    concept GLFWErrorCallback = tz::Action<F, int, const char*>;

    class Interface
    {
    public:
        template<GLFWErrorCallback CallbackT>
        void set_error_callback(CallbackT callback);

        friend void initialise(WindowCreationArgs args, WindowHintCollection hints); // Only this can initialise the Interface.
        friend void terminate(); // Only this can destroy the Interface;
        tz::IWindow& get_window();
    private:
        WindowImpl window;
        Interface(WindowCreationArgs args, WindowHintCollection hints);
        ~Interface() = default;
    };
}

#include "core/tz_glfw/interface.hpp"
#endif // TOPAZ_CORE_TZ_GLFW_INTERFACE_HPP