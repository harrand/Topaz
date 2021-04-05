namespace tz::ext::glfw
{
    template<GLFWErrorCallback CallbackT>
    void Interface::set_error_callback(CallbackT callback)
    {
        glfwSetErrorCallback(callback);
    }
}