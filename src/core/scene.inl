template<class Element, typename... Args>
Element& Scene::emplace(Args&&... args)
{
    if constexpr(std::is_same<Element, StaticObject>::value)
    {
        return emplace_object(std::forward<Args>(args)...);
    }
    else if constexpr(std::is_base_of_v<StaticObject, Element>)
    {
        this->heap_objects.push_back(std::make_unique<Element>(std::forward<Args>(args)...));
        return *(dynamic_cast<Element*>(this->heap_objects.back().get()));
    }
    else if constexpr(std::is_same<Element, Sprite>::value)
    {
        return emplace_sprite(std::forward<Args>(args)...);
    }
    else if constexpr(std::is_base_of_v<Sprite, Element>)
    {
        this->heap_sprites.push_back(std::make_unique<Element>(std::forward<Args>(args)...));
        return *(dynamic_cast<Element*>(this->heap_sprites.back().get()));
    }
    else
    {
        static_assert(std::is_void<Element>::value, "[Topaz Scene]: Scene::emplace has unsupported type.");
        // Do not need to return anything here as a compile-error will be emitted after the static_assertation.
    }
}

template<typename... Args>
StaticObject& Scene::emplace_object(Args&&... args)
{
    //return this->stack_objects.emplace_back(std::forward<Args>(args)...);
    this->heap_objects.push_back(std::make_unique<StaticObject>(std::forward<Args>(args)...));
    return *(this->heap_objects.back().get());
}

template<typename... Args>
Sprite& Scene::emplace_sprite(Args&&... args)
{
    this->heap_sprites.push_back(std::make_unique<Sprite>(std::forward<Args>(args)...));
    return *(this->heap_sprites.back().get());
}