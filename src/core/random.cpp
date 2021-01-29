#include "core/random.hpp"

namespace tz
{
    namespace detail
    {
        std::random_device rand_dev;

        std::random_device& device()
        {
            return rand_dev;
        }
    }
}