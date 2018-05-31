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
    return *dynamic_cast<GUIType*>((*result_pair.first).get());
}