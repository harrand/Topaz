template<class Element, typename... Args>
Element& Scene::emplace(Args&&... args)
{
	if constexpr(std::is_same<Element, Object>::value)
	{
		tz::util::log::message("emplaced normal object.");
		return emplace_object(std::forward<Args>(args)...);
	}
	else if constexpr(std::is_base_of_v<Object, Element>)
	{
		/// Element is inherited from Object so should be emplaced via heap
		tz::util::log::message("emplaced polymorphic object.");
		//std::unique_ptr<Element> ptr = ;
		this->heap_objects.push_back(std::make_shared<Element>(std::forward<Args>(args)...));
		return *(dynamic_cast<Element*>(this->heap_objects.back().get()));
	}
	else if constexpr(std::is_same<Element, Entity>::value)
	{
		return emplace_entity(std::forward<Args>(args)...);
	}
	else if constexpr(std::is_same<Element, EntityObject>::value)
	{
		return emplace_entity_object(std::forward<Args>(args)...);
	}
	else
	{
		static_assert(std::is_void<Element>::value, "[Topaz Scene]: Scene::emplace has unsupported type.");
		// Do not need to return anything here as a compile-error will be emitted after the static_assertation.
	}
}

template<typename... Args>
Object& Scene::emplace_object(Args&&... args)
{
	return this->objects.emplace_back(std::forward<Args>(args)...);
}

template<typename... Args>
Entity& Scene::emplace_entity(Args&&... args)
{
	return this->entities.emplace_back(std::forward<Args>(args)...);
}

template<typename... Args>
EntityObject& Scene::emplace_entity_object(Args&&... args)
{
	return this->entity_objects.emplace_back(std::forward<Args>(args)...);
}