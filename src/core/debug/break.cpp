#include "core/debug/break.hpp"
#include "debugbreak.h"

namespace tz
{
    void debugbreak()
    {
        debug_break();
    }
}