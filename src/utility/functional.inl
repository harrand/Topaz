namespace tz::utility::functional
{

}

template<typename FunctorT>
Functor<FunctorT>::Functor(FunctorT functor): functor(functor){}

template<typename FunctorT>
template<typename... FunctorParameters>
void Functor<FunctorT>::operator()(FunctorParameters... parameters)
{
    this->functor(parameters...);
}