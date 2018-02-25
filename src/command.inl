#include <thread>
#include <chrono>

template<typename Functor>
TrivialFunctor<Functor>::TrivialFunctor(Functor functor): functor(functor){}

template<typename Functor>
void TrivialFunctor<Functor>::operator()()
{
    this->functor();
}

namespace tz::util::scheduler
{
	template<typename Functor>
	inline void sync_delayed_functor(unsigned int millis_delay, TrivialFunctor<Functor> command)
	{
		std::this_thread::sleep_for(std::chrono::duration<unsigned int, std::milli>(millis_delay));
		command();
	}
	
	template<typename Functor>
	inline void async_delayed_functor(unsigned int millis_delay, TrivialFunctor<Functor> command)
	{
		std::thread(tz::util::scheduler::sync_delayed_functor<Functor>, millis_delay, command).detach();
	}
}
