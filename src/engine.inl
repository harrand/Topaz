template<typename Functor>
TrivialFunctor<Functor>* Engine::emplace_trivial_update_command(Functor&& functor)
{
    return this->update_command_executor.emplace_trivial_command(std::forward<Functor>(functor));
}

template<typename Functor, typename... FunctorParameters>
StaticFunctor<Functor, FunctorParameters...>* Engine::emplace_static_update_command(Functor&& functor, FunctorParameters&&... parameters)
{
    return this->update_command_executor.emplace_static_command(std::forward<Functor>(functor), std::forward<FunctorParameters>(parameters)...);
}

template<typename Functor>
TrivialFunctor<Functor>* Engine::emplace_trivial_tick_command(Functor&& functor)
{
    return this->tick_command_executor.emplace_trivial_command(std::forward<Functor>(functor));
}

template<typename Functor, typename... FunctorParameters>
StaticFunctor<Functor, FunctorParameters...>* Engine::emplace_static_tick_command(Functor&& functor, FunctorParameters&&... parameters)
{
    return this->tick_command_executor.emplace_static_command(std::forward<Functor>(functor), std::forward<FunctorParameters>(parameters)...);
}