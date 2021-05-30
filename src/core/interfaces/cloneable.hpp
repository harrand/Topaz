#ifndef TOPAZ_CORE_INTERFACES_CLONEABLE_HPP
#define TOPAZ_CORE_INTERFACES_CLONEABLE_HPP
#include <memory>

namespace tz
{
    template<typename T>
    class IUniqueCloneable
    {
    public:
        [[nodiscard]] virtual std::unique_ptr<T> unique_clone() const = 0;
    };
}

#endif //TOPAZ_CORE_INTERFACES_CLONEABLE_HPP