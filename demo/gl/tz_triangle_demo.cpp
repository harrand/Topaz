#include "core/tz.hpp"
#include "gl/device.hpp"

int main()
{
    constexpr tz::GameInfo tz_triangle_demo{"tz_triangle_demo", tz::EngineInfo::Version{1, 0, 0}, tz::info()};
    tz::initialise(tz_triangle_demo);
    {
        tz::gl::Device device;
        while(!tz::window().is_close_requested())
        {
            tz::window().update();
        }
    }
    tz::terminate();
}