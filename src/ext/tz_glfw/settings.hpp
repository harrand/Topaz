#ifndef TOPAZ_CORE_TZ_GLFW_SETTINGS_HPP
#define TOPAZ_CORE_TZ_GLFW_SETTINGS_HPP
#include <utility>

namespace tz::ext::glfw
{
    std::pair<int, int> get_monitor_dimensions();

    int get_monitor_width();
    int get_monitor_height();
}

#endif // TOPAZ_CORE_TZ_GLFW_SETTINGS_HPP