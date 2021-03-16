#include <utility>

namespace tz::mem
{
    template<typename Base, typename... Deriveds>
    constexpr PolymorphicVariant<Base, Deriveds...>::PolymorphicVariant(std::nullptr_t): buf{}, clean_ptr{nullptr}{}
}