template<class Element, typename... Args>
Element& Scene::emplace(Args&&... args)
{
    if constexpr(std::is_same<Element, SceneObject>::value)
    {
        return emplace_object(std::forward<Args>(args)...);
    }
    else if constexpr(std::is_base_of_v<SceneObject, Element>)
    {
        this->heap_objects.push_back(std::make_shared<Element>(std::forward<Args>(args)...));
        return *(dynamic_cast<Element*>(this->heap_objects.back().get()));
    }
    else
    {
        static_assert(std::is_void<Element>::value, "[Topaz Scene]: Scene::emplace has unsupported type.");
        // Do not need to return anything here as a compile-error will be emitted after the static_assertation.
    }
}

template<typename... Args>
SceneObject& Scene::emplace_object(Args&&... args)
{
    this->heap_objects.push_back(std::make_shared<SceneObject>(std::forward<Args>(args)...));
    return *(this->heap_objects.back().get());
}