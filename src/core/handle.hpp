#ifndef TOPAZ_CORE_OPAQUE_HANDLE_HPP
#define TOPAZ_CORE_OPAQUE_HANDLE_HPP
#include <cstddef>

namespace tz
{
    enum class HandleValue : std::size_t{};
    using HandleValueUnderlying = std::underlying_type_t<HandleValue>;
    struct nullhand_t{};
    constexpr nullhand_t nullhand;

    template<typename T>
    class Handle
    {
    public:
        Handle(HandleValue value):
        value(value){}

        Handle(nullhand_t):
        value(static_cast<HandleValue>(0)){}

        explicit operator HandleValue() const
        {
            return this->value;
        }

        Handle& operator=(HandleValue value)
        {
            this->value = value;
            return *this;
        }
        bool operator==(nullhand_t) const
        {
            return this->value == static_cast<HandleValue>(0);
        }
    private:
        HandleValue value;
    };
}

#endif //TOPAZ_CORE_OPAQUE_HANDLE_HPP