#ifndef TOPAZ_FUNCTIONAL_HPP
#define TOPAZ_FUNCTIONAL_HPP
#include <functional>

namespace tz::utility::functional
{
    using ButtonCallbackFunction = std::function<void()>;
}

/**
 * Wrapper for a Functor, using variadic arguments.
 * @tparam FunctorT - Type of the functor
 */
template<typename FunctorT>
class Functor
{
public:
    /**
     * Generate a Functor directly from a callable type.
     * @param functor - The functor value
     */
    Functor(FunctorT functor);
    /**
     * Execute the functor, providing all parameter values
     * @tparam FunctorParameters - Types of the functor parameters
     * @param parameters - Values of the functor parameters
     */
    template<typename... FunctorParameters>
    void operator()(FunctorParameters... parameters);
private:
    /// The underlying functor.
    FunctorT functor;
};

#include "functional.inl"
#endif //TOPAZ_FUNCTIONAL_HPP
