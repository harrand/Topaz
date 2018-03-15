#include <thread>
#include <chrono>

template<typename Functor>
TrivialFunctor<Functor>::TrivialFunctor(Functor&& functor): functor(std::forward<Functor>(functor)){}

template<typename Functor>
void TrivialFunctor<Functor>::operator()()
{
    this->functor();
}

// Perform perfect variadic forwarding to prevent reference collapsing into invalidity.
template<typename Functor, typename... FunctorParameters>
StaticFunctor<Functor, FunctorParameters...>::StaticFunctor(Functor&& functor, FunctorParameters&&... parameters): functor(std::forward<Functor>(functor)), parameters(std::tie(std::forward<FunctorParameters>(parameters)...)){}

template<typename Functor, typename... FunctorParameters>
void StaticFunctor<Functor, FunctorParameters...>::operator()()
{
    // std::apply can unpack the std::tuple (parameters) into the template parameter pack to pass into the functor parameters. It's a C++17 feature.
    std::apply(this->functor, this->parameters);
}

//todo
template<typename Functor>
TrivialFunctor<Functor>* CommandExecutor::emplace_trivial_command(Functor&& functor)
{
    this->owned_commands.push_back(std::make_unique<TrivialFunctor<Functor>>(std::forward<Functor>(functor)));
    return dynamic_cast<TrivialFunctor<Functor>*>(this->owned_commands.back().get());
}

template<typename Functor, typename... FunctorParameters>
StaticFunctor<Functor, FunctorParameters...>* CommandExecutor::emplace_static_command(Functor&& functor, FunctorParameters&&... parameters)
{
	this->owned_commands.push_back(std::make_unique<StaticFunctor<Functor, FunctorParameters...>>(std::forward<Functor>(functor), std::forward<FunctorParameters>(parameters)...));
	return dynamic_cast<StaticFunctor<Functor, FunctorParameters...>*>(this->owned_commands.back().get());
}

namespace tz::util::scheduler
{
	template<typename Functor>
	inline void sync_delayed_functor(unsigned int millis_delay, const TrivialFunctor<Functor>& command)
	{
		std::this_thread::sleep_for(std::chrono::duration<unsigned int, std::milli>(millis_delay));
		command();
	}
	
	template<typename Functor>
	inline void async_delayed_functor(unsigned int millis_delay, const TrivialFunctor<Functor>& command)
	{
		std::thread(tz::util::scheduler::sync_delayed_functor<Functor>, millis_delay, command).detach();
	}
}
