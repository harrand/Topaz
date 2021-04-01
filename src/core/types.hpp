#ifndef TOPAZ_CORE_TYPES_HPP
#define TOPAZ_CORE_TYPES_HPP
#include <type_traits>

namespace tz
{
    /**
     * An integral type or a floating-point type (including char and bool).
     */
    template<typename T>
    concept Arithmetic = std::is_arithmetic_v<T>;

    /**
     * A number is any arithmetic type excluding char and bool.
     */
    template<typename T>
    concept Number = requires
    {
        requires Arithmetic<T>;
        requires !std::is_same_v<std::remove_cvref_t<T>, bool>;
        requires !std::is_same_v<std::remove_cvref_t<T>, char>;
    };

    template<typename F, typename Result, typename... Args>
    concept Function = requires(F f, Args... args)
    {
        {f(args...)} -> std::convertible_to<Result>;
    };

    template<typename F, typename... Args>
    concept Action = Function<F, void, Args...>;
}

#endif // TOPAZ_CORE_TYPES_HPP