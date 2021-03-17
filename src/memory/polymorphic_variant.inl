#include <utility>

namespace tz::mem
{
    template<typename Base, typename... Deriveds>
    constexpr PolymorphicVariant<Base, Deriveds...>::PolymorphicVariant(std::nullptr_t): buf{}, clean_ptr{nullptr}{}

    template<typename Base, typename... Deriveds>
    PolymorphicVariant<Base, Deriveds...>::operator Base*()
    {
        return this->clean_ptr;
    }

    template<typename Base, typename... Deriveds>
    PolymorphicVariant<Base, Deriveds...>::operator const Base*() const
    {
        return this->clean_ptr;
    }

    template<typename Base, typename... Deriveds>
    Base& PolymorphicVariant<Base, Deriveds...>::operator->()
    {
        topaz_assert(this->clean_ptr != nullptr, "tz::mem::PolymorphicVariant<...>::operator->(): This was nullptr.");
        return this->clean_ptr;
    }

    template<typename Base, typename... Deriveds>
    const Base& PolymorphicVariant<Base, Deriveds...>::operator->() const
    {
        topaz_assert(this->clean_ptr != nullptr, "tz::mem::PolymorphicVariant<...>::operator->() const: This was nullptr.");
        return this->clean_ptr;
    }

    template<typename Base, typename... Deriveds>
    Base& PolymorphicVariant<Base, Deriveds...>::operator*()
    {
        topaz_assert(this->clean_ptr != nullptr, "tz::mem::PolymorphicVariant<...>::operator*(): This was nullptr.");
        return *this->clear_ptr;
    }

    template<typename Base, typename... Deriveds>
    const Base& PolymorphicVariant<Base, Deriveds...>::operator*() const
    {
        topaz_assert(this->clean_ptr != nullptr, "tz::mem::PolymorphicVariant<...>::operator*() const: This was nullptr.");
        return *this->clear_ptr;
    }

    template<typename Base, typename... Deriveds>
    bool PolymorphicVariant<Base, Deriveds...>::has_value() const
    {
        return this->clean_ptr != nullptr;
    }

    template<typename Base, typename... Deriveds>
    template<typename Derived>
    PolymorphicVariant<Base, Deriveds...>& PolymorphicVariant<Base, Deriveds...>::operator=(Derived&& d)
    {
        this->clean_ptr = new (this->buf) Derived{d};
        return *this;
    }

    template<typename Base, typename... Deriveds>
    PolymorphicVariant<Base, Deriveds...>& PolymorphicVariant<Base, Deriveds...>::operator=(Base&& b)
    {
        static_assert(std::is_constructible_v<Base>(), "tz::mem::PolymorphicVariant::operator=(Base): Invoked, but Base is not constructible! Perhaps it's pure virtual?");
        this->clean_ptr = new (this->buf) Base{b};
        return *this;
    }

    template<typename Base, typename... Deriveds>
    PolymorphicVariant<Base, Deriveds...>& PolymorphicVariant<Base, Deriveds...>::operator=(std::nullptr_t)
    {
        this->clean_ptr = nullptr;
        return *this;
    }

    template<typename Base, typename... Deriveds>
    template<typename Derived, typename... Ts>
    void PolymorphicVariant<Base, Deriveds...>::emplace(Ts&&... ts)
    {
        this->clean_ptr = new (this->buf) Derived(std::forward<Ts>(ts)...);
    }
}