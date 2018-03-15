//todo
template<typename Functor>
TrivialFunctor<Functor>* Engine::emplace_trivial_update_command(Functor&& functor)
{
    return this->update_command_executor.emplace_trivial_command(std::forward<Functor>(functor));
}