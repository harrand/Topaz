template<class GUIType, typename... Args>
GUIType& Window::emplace_child(Args&&... args)
{
    // result_pair is <iterator, bool>
    std::shared_ptr<GUIType> gui_shared = std::make_shared<GUIType>(std::forward<Args>(args)...);
    this->window_gui_element.heap_children.push_back(gui_shared);
    gui_shared->parent = &this->window_gui_element;
    this->conditionally_register_additional_listeners<GUIType>(gui_shared.get());
    return *gui_shared.get();
}

namespace tz::detail
{
    // SFINAE test
    template <typename T>
    class HasGetMouseListener
    {
        typedef char one;
        typedef long two;

        template <typename C> static one test( decltype(&C::get_mouse_listener) ){return {};}
        template <typename C> static two test(...){return {};}

    public:
        enum { value = sizeof(test<T>(0)) == sizeof(char) };
    };

    template <typename T>
    class HasGetKeyListener
    {
        typedef char one;
        typedef long two;

        template <typename C> static one test( decltype(&C::get_key_listener) ){return {};}
        template <typename C> static two test(...){return {};}

    public:
        enum { value = sizeof(test<T>(0)) == sizeof(char) };
    };
}

template<typename GUIType>
void Window::dispose_child([[maybe_unused]] GUIType *gui)
{
    if constexpr(tz::detail::HasGetKeyListener<GUIType>::value)
    {
        if(gui->key_sensitive())
            this->deregister_listener(*gui->get_key_listener());
    }

    if constexpr(tz::detail::HasGetMouseListener<GUIType>::value)
    {
        if(gui->mouse_sensitive())
            this->deregister_listener(*gui->get_mouse_listener());
    }
}

template<class GUIType>
void Window::conditionally_register_additional_listeners([[maybe_unused]] GUIType *gui_type)
{
    if constexpr(tz::detail::HasGetKeyListener<GUIType>::value)
    {
        if(gui_type->key_sensitive())
            this->register_listener(*gui_type->get_key_listener());
    }

    if constexpr(tz::detail::HasGetMouseListener<GUIType>::value)
    {
        if(gui_type->mouse_sensitive())
            this->register_listener(*gui_type->get_mouse_listener());
    }

}