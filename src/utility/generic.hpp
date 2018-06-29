#ifndef GENERIC_UTILITY_HPP
#define GENERIC_UTILITY_HPP

#include <string>
#include <cstddef>
#include "numeric.hpp"

namespace tz::utility::generic
{
    /**
     * Given a container, find the size, in bytes, of an element in the container.
	 * @tparam Container - Container type.
	 * @param element_list - Container of elements.
	 * @return
	 */
    template<typename Container>
    constexpr std::size_t sizeof_element(Container element_list);

    namespace literals
    {
        /**
        * Convert a mass in metric kilograms (kg) to imperial pounds (lb).
        */
        long double operator""_lb(long double mass);
        /**
        * Convert a mass in metric kilograms(kg) to imperial stone (st).
        */
        long double operator""_st(long double mass);
        /**
        * Convert an angle in degrees to an angle in radians.
        * i.e: 180_deg = π
        */
        inline long double operator""_deg(long double angle);
        /**
        * Convert an angle in radians to an angle in degrees.
        * i.e: π_rad = 180
        */
        inline long double operator""_rad(long double angle)
        {
            return angle * 180.0 / tz::utility::numeric::consts::pi;
        }
    }

    namespace cast
    {
        /**
			 * (Attempt to) Convert an object to a std::string.
			 * @tparam T - Type of the object to convert
			 * @param obj - The object to convert
			 * @return - The object, converted to a string
			 */
        template <typename T>
        std::string to_string(T&& obj);
        /**
         * (Attempt to) Convert an std::string to an object of specified type.
         * @tparam T - Type of the object to convert to
         * @param s - The object to convert to
         * @return - The object that the string was converted to
         */
        template <typename T>
        T from_string(const std::string& s);
    }
}

#include "generic.inl"

#endif //TOPAZ_GENERIC_HPP
