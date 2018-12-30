namespace tz::utility::functional
{
    template<typename A, typename B>
    constexpr bool is()
    {
        return std::is_same_v<A, B>;
    }

    template<typename Base, typename Derived>
    constexpr bool is_a(const Derived& derived)
    {
        return dynamic_cast<Base*>(&derived) != nullptr;
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