#ifndef TOPAZ_MEMORY_POLYMORPHIC_VARIANT_HPP
#define TOPAZ_MEMORY_POLYMORPHIC_VARIANT_HPP
#include "memory/polymorphic/base.hpp"

namespace tz::mem
{
    /**
	 * \addtogroup tz_mem Topaz Memory Library (tz::mem)
	 * A collection of low-level abstractions around memory utilities not provided by the C++ standard library. This includes non-owning memory blocks, uniform memory-pools and more.
	 * @{
	 */

    /**
     * Creates local storage large enough to hold any of the derived types and returns a base pointer. The purpose of this is cache coherency -- While the additional line reduces cache line availability, having a parent struct non-fragmented may lead to a performance win.
     * 
     * If the set of all known supported derived types is known, this can be used to own a polymorphic object without allocation.
     * Note: sizeof(PolymorphicVariant<A, B...>) >= Largest sizeof A or any element within B... If derived types vary wildly in size, you are likely to waste quite a bit of stack space.
     */
    template<typename Base, typename... Deriveds>
    class PolymorphicVariant
    {
    public:
        constexpr PolymorphicVariant(std::nullptr_t = nullptr);
        PolymorphicVariant(const PolymorphicVariant& copy) = delete;
        PolymorphicVariant(PolymorphicVariant&& move) = delete;
        template<typename Derived>
        constexpr PolymorphicVariant(Derived&& derived);

        operator Base*();
        operator const Base*() const;
        Base& operator->();
        const Base& operator->() const;
        Base& operator*();
        const Base& operator*() const;

        bool has_value() const;

        template<typename Derived>
        PolymorphicVariant& operator=(Derived&& d);
        PolymorphicVariant& operator=(Base&& b);
        PolymorphicVariant& operator=(std::nullptr_t);

        template<typename Derived, typename... Ts>
        void emplace(Ts&&... ts);
    private:
        void destruct_current();

        std::aligned_storage_t<tz::mem::max_sizeof<Base, Deriveds...>(), tz::mem::max_alignof<Base, Deriveds...>()> buf;
        Base* clean_ptr;
    };

    /**
     * }@
     */
}

#include "memory/polymorphic_variant.inl"
#endif // TOPAZ_MEMORY_POLYMORPHIC_VARIANT_HPP