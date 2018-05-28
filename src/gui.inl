#include <utility>

template<class GUIType, typename... Args>
GUIType* GUI::emplace_child(Args&&... args)
{
    // result_pair is <iterator, bool>
    auto result_pair = this->heap_children.emplace(std::make_shared<GUIType>(std::forward<Args>(args)...));
    if(!result_pair.second)
    {
        std::cerr << "Attempt to emplace child into GUI element failed.\n";
        return nullptr;
    }
    result_pair.first->get()->parent = this;
    return dynamic_cast<GUIType*>((*result_pair.first).get());
}