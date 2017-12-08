template<typename Functor>
TrivialFunctor<Functor>::TrivialFunctor(Functor functor): functor(functor){}

template<typename Functor>
void TrivialFunctor<Functor>::operator()()
{
	this->functor();
}