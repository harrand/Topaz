#include <sstream>

namespace tz::utility::generic
{
    template<typename Container>
    constexpr std::size_t sizeof_element(Container element_list)
    {
        return sizeof(typename decltype(element_list)::value_type);
    }

    namespace cast
    {
        template <typename T>
        std::string to_string(T&& obj)
        {
            if constexpr(std::is_convertible_v<T, std::string>)
            {
                return std::forward<T>(obj);
            }
            else if constexpr(std::is_same<T, int>::value || std::is_same<T, long>::value || std::is_same<T, long long>::value || std::is_same<T, unsigned>::value || std::is_same<T, unsigned long>::value || std::is_same<T, unsigned long long>::value || std::is_same<T, float>::value || std::is_same<T, double>::value || std::is_same<T, long double>::value)
            {// runs if obj is a valid parameter for std::to_string(...)
                return std::to_string(std::forward<T>(obj));
            }
            std::ostringstream oss;
            oss << std::forward<T>(obj);
            return oss.str();
        }

        template <typename T>
        T from_string(const std::string& s)
        {
            static_assert(std::is_trivially_constructible_v<T>, "tz::util::cast::from_string<T>'s return type T must be trivially constructible.");
            T ret;
            std::istringstream ss(s);
            ss >> ret;
            return ret;
        }
    }
}