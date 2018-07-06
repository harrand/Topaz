
template<class GUIType, typename... Args>
GUIType& Window::emplace_child(Args&&... args)
{
    // result_pair is <iterator, bool>
    auto result_pair = this->window_gui_element.heap_children.emplace(std::make_shared<GUIType>(std::forward<Args>(args)...));
    if(!result_pair.second)
    {
        std::cerr << "GUI element already contained this child, returning that instead of inserting a duplicate.\n";
        return *dynamic_cast<GUIType*>((*result_pair.first).get());
    }
    result_pair.first->get()->parent = &this->window_gui_element;
    GUIType* emplaced_child = dynamic_cast<GUIType*>((*result_pair.first).get());
    this->conditionally_register_additional_listeners<GUIType>(emplaced_child);
    return *emplaced_child;
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

template<class GUIType>
void Window::conditionally_register_additional_listeners([[maybe_unused]] GUIType *gui_type)
{
    if constexpr(tz::detail::HasGetKeyListener<GUIType>::value)
    {
        if(gui_type->key_sensitive())
            this->register_listener(gui_type->key_listener.value());
    }

    if constexpr(tz::detail::HasGetMouseListener<GUIType>::value)
    {
        if(gui_type->mouse_sensitive())
            this->register_listener(gui_type->mouse_listener.value());
    }

}