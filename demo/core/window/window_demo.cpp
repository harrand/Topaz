//
// Created by Harrand on 13/12/2019.
//

#include "core/core.hpp"
#include "window_demo.hpp"
#include <iostream>

int main(int argc, char** argv)
{
    tz::core::initialise("Topaz -- WindowDemo");
    printf("%s", "Hello World from WindowDemo!\n");
    std::cout << "how about stdcout?\n";
    tz::core::terminate();
    return 0;
}