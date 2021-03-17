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
        /**
         * Construct an empty PolymorphicVariant. It's essentially a nullptr.
         */
        constexpr PolymorphicVariant(std::nullptr_t = nullptr);
        PolymorphicVariant(const PolymorphicVariant& copy) = delete;
        PolymorphicVariant(PolymorphicVariant&& move) = delete;
        /**
         * Construct a PolymorphicVariant via an existing derived type value.
         * Precondition: Derived is among the initially listed set of available derived types (the Deriveds... template)
         * @tparam Derived Derived type to set.
         * @param derived Universal reference used to construct the new derived value.
         */
        template<typename Derived>
        constexpr PolymorphicVariant(Derived&& derived);

        /**
         * Interpret the PolymorphicVariant as its base.
         * @return Base* referring to the stored value.
         */
        operator Base*();
        /**
         * Interpret the PolymorphicVariant as its base.
         * @return Base* referring to the stored value.
         */
        operator const Base*() const;
        /**
         * Interpret the PolymorphicVariant as its base.
         * Precondition: A stored value exists within this variant. In other words, this->has_value() is true. Otherwise this will assert and invoke UB.
         * @return Base& referring to the stored value.
         */
        Base& operator->();
        /**
         * Interpret the PolymorphicVariant as its base.
         * Precondition: A stored value exists within this variant. In other words, this->has_value() is true. Otherwise this will assert and invoke UB.
         * @return Base& referring to the stored value.
         */
        const Base& operator->() const;
        /**
         * Interpret the PolymorphicVariant as its base.
         * Precondition: A stored value exists within this variant. In other words, this->has_value() is true. Otherwise this will assert and invoke UB.
         * @return Base& referring to the stored value.
         */
        Base& operator*();
        /**
         * Interpret the PolymorphicVariant as its base.
         * Precondition: A stored value exists within this variant. In other words, this->has_value() is true. Otherwise this will assert and invoke UB.
         * @return Base& referring to the stored value.
         */
        const Base& operator*() const;
        /**
         * Query as to whether there is a stored value within this variant.
         * @return True if a value exists, otherwise false.
         */
        bool has_value() const;
        /**
         * Assign the underlying value to a derived type value.
         * Precondition: Derived is among the initially listed set of available derived types (the Deriveds... template)
         * @tparam Derived Derived type to set.
         * @param d Derived value to set.
         */
        template<typename Derived>
        PolymorphicVariant& operator=(Derived&& d);
        /**
         * Assign the underlying value to a base type value.
         * Precondition: Base is a constructible type.
         * @param b Base value to set.
         */
        PolymorphicVariant& operator=(Base&& b);
        /**
         * Assign the underlying value to nullptr, meaning that there is no underlying value. After invoking this, this->has_value() will return false.
         */
        PolymorphicVariant& operator=(std::nullptr_t);
        /**
         * Construct a Derived type in-place within the variant. Can be used to skip unnecessary constructor invocations.
         * @tparam Derived Derived type to set.
         * @tparam Ts... Element types used to invoke Derived's constructor.
         * @param ts... Element values used to invoke Derived's constructor.
         */
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