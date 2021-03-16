#ifndef TOPAZ_MEMORY_POLYMORPHIC_VARIANT_HPP
#define TOPAZ_MEMORY_POLYMORPHIC_VARIANT_HPP
#include "memory/polymorphic/base.hpp"

namespace tz::mem
{
    template<typename Base, typename... Deriveds>
    class PolymorphicVariant
    {
    public:
        constexpr PolymorphicVariant(std::nullptr_t);

        operator Base*()
        {
            return this->clean_ptr;
        }
        operator const Base*() const
        {
            return this->clean_ptr;
        }
        Base* operator->()
        {
            topaz_assert(this->clean_ptr != nullptr, "tz::mem::PolymorphicVariant<...>::operator->(): This was nullptr.");
            return this->clean_ptr;
        }
        const Base* operator->() const
        {
            topaz_assert(this->clean_ptr != nullptr, "tz::mem::PolymorphicVariant<...>::operator->() const: This was nullptr.");
            return this->clean_ptr;
        }
        Base& operator*()
        {
            topaz_assert(this->clean_ptr != nullptr, "tz::mem::PolymorphicVariant<...>::operator*(): This was nullptr.");
            return *this->clear_ptr;
        }
        const Base& operator*() const
        {
            topaz_assert(this->clean_ptr != nullptr, "tz::mem::PolymorphicVariant<...>::operator*() const: This was nullptr.");
            return *this->clear_ptr;
        }

        template<typename Derived>
        PolymorphicVariant& operator=(Derived&& d)
        {
            this->clean_ptr = new (this->buf) Derived{d};
            return *this;
        }

        PolymorphicVariant& operator=(Base&& b)
        {
            static_assert(std::is_constructible_v<Base>(), "tz::mem::PolymorphicVariant::operator=(Base): Invoked, but Base is not constructible! Perhaps it's pure virtual?");
            this->clean_ptr = new (this->buf) Base{b};
            return *this;
        }

        PolymorphicVariant& operator=(std::nullptr_t)
        {
            this->clean_ptr = nullptr;
            return *this;
        }

        template<typename Derived, typename... Ts>
        void emplace(Ts&&... ts)
        {
            this->clean_ptr = new (this->buf) Derived(std::forward<Ts>(ts)...);
        }
    private:
        char buf[tz::mem::register_base<Base, Deriveds...>()];
        Base* clean_ptr;
    };
}

#include "memory/polymorphic_variant.inl"
#endif // TOPAZ_MEMORY_POLYMORPHIC_VARIANT_HPP