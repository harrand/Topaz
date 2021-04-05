#ifndef TOPAZ_CORE_TZ_GLFW_INTERFACE_HPP
#define TOPAZ_CORE_TZ_GLFW_INTERFACE_HPP
#include "core/types.hpp"
#include "ext/tz_glfw/window_impl.hpp"

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

    /**
     * Default GLFW error callback. Simply topaz_asserts on the error.
     */
    void default_error_callback(int error_code, const char* msg);

    /**
     * Represents the centre of communication between topaz modules and GLFW.
     */
    class Interface
    {
    public:
        /**
         * Set GLFW to use the given error callback. By default, this is tz::ext::glfw::default_error_callback.
         * @tparam CallbackT Callback type, can be any invocable matching the signature constrained by tz::ext::glfw::GLFWErrorCallback.
         * @param callback Callback value to be invoked when a GLFW error occurs.
         */
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

#include "ext/tz_glfw/interface.inl"
#endif // TOPAZ_CORE_TZ_GLFW_INTERFACE_HPP