#ifndef TOPAZ_FUNCTIONAL_HPP
#define TOPAZ_FUNCTIONAL_HPP
#include <functional>

namespace tz::utility::functional
{
    /// Simple alternative for a Runnable, used by GUI classes.
    using ButtonCallbackFunction = std::function<void()>;
    /**
     * Verifies type equality.
     * @tparam A - Type A
     * @tparam B - Type B
     * @return - True if A is equivalent to B. Otherwise false
     */
    template<typename A, typename B>
    constexpr bool is();
    /**
     * Verifies type equality or inheritance
     * @tparam A - Type A
     * @tparam B - Type B
     * @return - True if A is equivalent to B or is a parent class of B. False otherwise
     */
    template<typename A, typename B>
    constexpr bool is_related();
    /**
     * Verifies that a given base-class instance is also an instance of a given subclass.
     * @tparam Base - The base type and type of the parameter
     * @tparam Derived - The specific type deriving from Base
     * @param base - The instance of the base type
     * @return - True if base is actually a Derived. Otherwise false. Functionally equivalent to instanceof in Java
     */
    template<typename Base, typename Derived>
    constexpr bool is_a(const Base& base);
    /**
     * Wrapper for either std::ref or std::cref
     * @tparam T - The reference type
     * @param t - The given reference
     * @return - Reference wrapper equivalent to the given reference
     */
    template<typename T>
    std::reference_wrapper<T> build_reference(T& t);
    /**
     * Given a vector of references to a base class, extract all elements of the vector that are also instances of a given derived class.
     * @tparam Base - Base class and underlying type of the vector template reference
     * @tparam Derived - Desired Derived type of which to extract from the vector
     * @tparam ReferenceWrapper - Any type where ReferenceWrapper<T> is implicitly convertible to T&. This is std::reference_wrapper by default
     * @param container - Container of base class references
     * @param wrapper_generator - Function used to wrap the reference. This is tz::utility::functional::build_reference<Derived>(Derived&) by default
     * @return - Vector of references to the derived class
     */
    template<typename Base, typename Derived, template<typename> typename ReferenceWrapper = std::reference_wrapper>
    std::vector<ReferenceWrapper<Derived>> get_subclasses(const std::vector<ReferenceWrapper<Base>>& container, const std::function<ReferenceWrapper<Derived>(Derived&)>& wrapper_generator = tz::utility::functional::build_reference<Derived>);
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
    auto operator()(FunctorParameters&&... parameters) const;
private:
    /// The underlying functor.
    FunctorT functor;
};

using Runnable = Functor<std::function<void()>>;

#include "functional.inl"
#endif //TOPAZ_FUNCTIONAL_HPP