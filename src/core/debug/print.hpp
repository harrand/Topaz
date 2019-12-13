//
// Created by Harrand on 13/12/2019.
//

#ifndef TOPAZ_PRINT_HPP
#define TOPAZ_PRINT_HPP

namespace tz
{
    template<typename... Args>
    void debug_printf(const char* fmt, Args&&... args);
}

#include "core/debug/print.inl"
#endif //TOPAZ_PRINT_HPP
