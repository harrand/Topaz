#include <typeinfo>

template<class Element, typename... Args>
Element& Scene::emplace(Args&&... args)
{
    if constexpr(std::is_same<Element, Renderable>::value)
    {
        Element& result = emplace_object(std::forward<Args>(args)...);
        return result;
    }
    else if constexpr(std::is_base_of_v<Renderable, Element>)
    {
        this->objects.push_back(std::make_unique<Element>(std::forward<Args>(args)...));
        Element* result = dynamic_cast<Element*>(this->objects.back().get());
        this->octree.enqueue_object(result);
        this->inheritance_map.insert({typeid(result), dynamic_cast<Renderable*>(result)});
        return *result;
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
    this->objects.push_back(std::make_unique<StaticObject>(std::forward<Args>(args)...));
    StaticObject* result = dynamic_cast<StaticObject*>(this->objects.back().get());
    this->octree.enqueue_object(result);
    this->inheritance_map.insert({typeid(result), dynamic_cast<Renderable*>(result)});
    return *result;
}

template<typename... Args>
Sprite& Scene::emplace_sprite(Args&&... args)
{
    this->objects.push_back(std::make_unique<Sprite>(std::forward<Args>(args)...));
    return dynamic_cast<Sprite&>(*(this->objects.back().get()));
    /*
    this->heap_sprites.push_back(std::make_unique<Sprite>(std::forward<Args>(args)...));
    return *(this->heap_sprites.back().get());
     */
}

template<class RenderableType>
std::size_t Scene::get_number_of() const
{
    auto range = this->inheritance_map.equal_range(typeid(RenderableType*));
    return static_cast<std::size_t>(std::abs(std::distance(range.first, range.second)));
}

template<>
std::size_t Scene::get_number_of<Renderable>() const
{
    return this->objects.size();
}

template<class RenderableType>
SceneSection<RenderableType> Scene::get_renderables_by_type()
{
    static_assert(!std::is_base_of_v<RenderableType, Renderable>, "Scene::get_renderables_by_type<T>() has been invoked with T not being a subclass of Renderable. This is not allowed.");
    static_assert(!std::is_same_v<RenderableType, Renderable>, "Scene::get_renderables_by_type<Renderable>() is not possible. This only works on specific sub-classes of Renderable. Not Renderable itself. For a container of Renderables in the scene, use Scene::get_renderables()");
    auto range = this->inheritance_map.equal_range(typeid(RenderableType*));
    return {range.first, range.second};
}

template<class RenderableType>
SceneSection<const RenderableType> Scene::get_renderables_by_type() const
{
    auto range = this->inheritance_map.equal_range(typeid(const RenderableType*));
    return {range.first, range.second};
}

template<class RenderableType>
SceneSection<RenderableType>::iterator::iterator(SceneSection<RenderableType>::IteratorType iter): iter(iter){}

template<class RenderableType>
typename SceneSection<RenderableType>::iterator& SceneSection<RenderableType>::iterator::operator++()
{
    this->iter++;
    return *this;
}

template<class RenderableType>
typename SceneSection<RenderableType>::ValueType SceneSection<RenderableType>::iterator::operator*()
{
    return dynamic_cast<SceneSection::ValueType>(this->iter->second);
}

template<class RenderableType>
bool SceneSection<RenderableType>::iterator::operator!=(const SceneSection<RenderableType>::iterator &rhs) const
{
    return this->iter != rhs.iter;
}

template<class RenderableType>
SceneSection<RenderableType>::const_iterator::const_iterator(SceneSection<RenderableType>::ConstIteratorType iter): iter(iter){}

template<class RenderableType>
typename SceneSection<RenderableType>::const_iterator& SceneSection<RenderableType>::const_iterator::operator++()
{
    this->iter++;
    return *this;
}

template<class RenderableType>
typename SceneSection<RenderableType>::ConstValueType SceneSection<RenderableType>::const_iterator::operator*() const
{
    return dynamic_cast<SceneSection::ConstValueType>(this->iter->second);
}

template<class RenderableType>
bool SceneSection<RenderableType>::const_iterator::operator!=(const SceneSection<RenderableType>::const_iterator& rhs) const
{
    return this->iter != rhs.iter;
}

template<class RenderableType>
SceneSection<RenderableType>::SceneSection(SceneSection<RenderableType>::IteratorType begin, SceneSection<RenderableType>::IteratorType end): range(begin, end){}

template<class RenderableType>
std::size_t SceneSection<RenderableType>::size() const
{
    return static_cast<std::size_t>(std::abs(std::distance(this->range.first, this->range.second)));
}

template<class RenderableType>
typename SceneSection<RenderableType>::iterator SceneSection<RenderableType>::begin()
{
    return {this->range.first};
}

template<class RenderableType>
typename SceneSection<RenderableType>::const_iterator SceneSection<RenderableType>::cbegin() const
{
    return {const_cast<ConstIteratorType>(this->range.first)};
}

template<class RenderableType>
typename SceneSection<RenderableType>::iterator SceneSection<RenderableType>::end()
{
    return {this->range.second};
}

template<class RenderableType>
typename SceneSection<RenderableType>::const_iterator SceneSection<RenderableType>::cend() const
{
    return {const_cast<ConstIteratorType>(this->range.second)};
}