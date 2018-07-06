#include <utility>

template<class GUIType, typename... Args>
GUIType& GUI::emplace_child(Args&&... args)
{
    // result_pair is <iterator, bool>
    auto result_pair = this->heap_children.emplace(std::make_shared<GUIType>(std::forward<Args>(args)...));
    if(!result_pair.second)
    {
        std::cerr << "GUI element already contained this child, returning that instead of inserting a duplicate.\n";
        return *dynamic_cast<GUIType*>((*result_pair.first).get());
    }
    result_pair.first->get()->parent = this;
    return *dynamic_cast<GUIType*>((*result_pair.first).get());
}

template<template<typename> class Container>
std::unordered_set<GUI*> GUI::get_occluders(const Container<GUI*>& gui_elements)
{
    std::unordered_set<GUI*> occluders;
    Vector2I minimum = this->get_screen_position_pixel_space();
    Vector2I maximum = minimum + Vector2I{this->get_width(), this->get_height()};
    for(GUI* element : gui_elements)
    {
        Vector2I element_minimum = element->get_screen_position_pixel_space();
        Vector2I element_maximum = element_minimum + Vector2I{element->get_width(), element->get_height()};
        if(minimum > element_minimum && maximum < element_maximum)
            occluders.insert(element);
    }
    return occluders;
}

template<template<typename> class Container>
bool GUI::is_occluded_by(const Container<GUI*>& gui_elements)
{
    return this->get_occluders(gui_elements).size() != 0;
}