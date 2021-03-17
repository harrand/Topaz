#ifndef TOPAZ_MEM_POLYMORPHIC_BASE_HPP
#define TOPAZ_MEM_POLYMORPHIC_BASE_HPP
#include <array>
#include <cstddef>
#include "algo/static.hpp"

namespace tz::mem
{
    namespace detail
    {
        class Mixmax
        {
        public:
            constexpr Mixmax(): max_value(0){}

            constexpr void mix(int value)
            {
                this->max_value = std::max(value, this->max_value);
            }

            constexpr int max() const
            {
                return this->max_value;
            }
        private:
            int max_value;
        };

        template<typename... Types>
        constexpr auto get_types_size()
        {
            return std::array<std::size_t, sizeof...(Types)>{sizeof(Types)...};
        }

        template<typename... Types>
        constexpr auto get_types_alignment()
        {
            return std::array<std::size_t, sizeof...(Types)>{alignof(Types)...};
        }
    }

    template<typename Base, typename... DerivedTypes>
    constexpr int max_sizeof()
    {
        detail::Mixmax m;
        auto arr = detail::get_types_size<Base, DerivedTypes...>();
        tz::algo::static_for<0, arr.size()>([&m, arr](auto idx){m.mix(static_cast<int>(arr[idx]));});
        return m.max();
    }

    template<typename Base, typename... DerivedTypes>
    constexpr int max_alignof()
    {
        detail::Mixmax m;
        auto arr = detail::get_types_alignment<Base, DerivedTypes...>();
        tz::algo::static_for<0, arr.size()>([&m, arr](auto idx){m.mix(static_cast<int>(arr[idx]));});
        return m.max();
    }
}

#endif // TOPAZ_MEM_POLYMORPHIC_BASE_HPP