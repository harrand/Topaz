namespace tz::utility::functional
{
    template<typename A, typename B>
    constexpr bool is()
    {
        return std::is_same_v<A, B>;
    }

    template<typename Base, typename Derived>
    constexpr bool is_a(const Base& base)
    {
        return dynamic_cast<const Derived*>(&base) != nullptr;
    }

    template<typename T>
    std::reference_wrapper<T> build_reference(T& t)
    {
        if constexpr(std::is_const_v<T>)
            return std::cref(t);
        else
            return std::ref(t);
    }

    template<typename Base, typename Derived, template<typename> typename ReferenceWrapper = std::reference_wrapper>
    std::vector<ReferenceWrapper<Derived>> get_subclasses(const std::vector<ReferenceWrapper<Base>>& container, const std::function<ReferenceWrapper<Derived>(Derived&)>& wrapper_generator)
    {
        std::vector<ReferenceWrapper<Derived>> subclasses;
        for(Base& base : container)
            if(dynamic_cast<Derived*>(&base) != nullptr)
                subclasses.push_back(wrapper_generator(dynamic_cast<Derived&>(base)));
        return subclasses;
    }
}

template<typename FunctorT>
Functor<FunctorT>::Functor(FunctorT functor): functor(functor){}

template<typename FunctorT>
template<typename... FunctorParameters>
auto Functor<FunctorT>::operator()(FunctorParameters&&... parameters) const
{
    return this->functor(std::forward<FunctorParameters>(parameters)...);
}