#include "core/core.hpp"
#include "gl/frame.hpp"

int main()
{
    tz::core::initialise("Topaz Window Demo");
    {
        tz::core::IWindow& window = tz::core::get().window();

        while(!window.is_close_requested())
        {
            window.get_frame()->clear();
            // Do stuff.
            tz::core::update();
            window.update();
        }
        
    }
    tz::core::terminate();
    return 0;
}