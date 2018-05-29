template<class GUIType, typename... Args>
GUIType* Window::emplace_child(Args&&... args)
{
    // result_pair is <iterator, bool>
    auto result_pair = this->window_gui_element.heap_children.emplace(std::make_shared<GUIType>(std::forward<Args>(args)...));
    if(!result_pair.second)
    {
        std::cerr << "Attempt to emplace child into GUI element failed.\n";
        return nullptr;
    }
    result_pair.first->get()->parent = &this->window_gui_element;
    return dynamic_cast<GUIType*>((*result_pair.first).get());
}