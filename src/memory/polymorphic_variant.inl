#include <utility>

namespace tz::mem
{
    template<typename Base, typename... Deriveds>
    constexpr PolymorphicVariant<Base, Deriveds...>::PolymorphicVariant(std::nullptr_t): buf{}, clean_ptr{nullptr}{}

    template<typename Base, typename... Deriveds>
    template<typename Derived>
    constexpr PolymorphicVariant<Base, Deriveds...>::PolymorphicVariant(Derived&& derived)
    {
        static_assert(tz::algo::static_find<Derived, Deriveds...>(), "tz::mem::PolymorphicVariant: Construction from derived type that is not known by the variant.");
        this->clean_ptr = new (&this->buf) Derived{std::forward<Derived>(derived)};
    }

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
        using DerivedDecayed = std::decay_t<Derived>;
        this->destruct_current();
        this->clean_ptr = new (&this->buf) DerivedDecayed{std::forward<Derived>(d)};
        return *this;
    }

    template<typename Base, typename... Deriveds>
    PolymorphicVariant<Base, Deriveds...>& PolymorphicVariant<Base, Deriveds...>::operator=(Base&& b)
    {
        static_assert(std::is_constructible_v<Base>(), "tz::mem::PolymorphicVariant::operator=(Base): Invoked, but Base is not constructible! Perhaps it's pure virtual?");
        this->destruct_current();
        this->clean_ptr = new (&this->buf) Base{std::forward<Base>(b)};
        return *this;
    }

    template<typename Base, typename... Deriveds>
    PolymorphicVariant<Base, Deriveds...>& PolymorphicVariant<Base, Deriveds...>::operator=(std::nullptr_t)
    {
        this->destruct_current();
        this->clean_ptr = nullptr;
        return *this;
    }

    template<typename Base, typename... Deriveds>
    template<typename Derived, typename... Ts>
    void PolymorphicVariant<Base, Deriveds...>::emplace(Ts&&... ts)
    {
        this->destruct_current();
        this->clean_ptr = new (&this->buf) Derived(std::forward<Ts>(ts)...);
    }

    template<typename Base, typename... Deriveds>
    void PolymorphicVariant<Base, Deriveds...>::destruct_current()
    {
        if(this->has_value())
        {
            this->clean_ptr->~Base();
        }
    }
}