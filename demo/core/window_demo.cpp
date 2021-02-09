#include "core/tz.hpp"
#include "gl/frame.hpp"

int main()
{
    tz::initialise("Topaz Window Demo");
    {
        tz::IWindow& window = tz::get().window();

        while(!window.is_close_requested())
        {
            window.get_frame()->clear();
            // Do stuff.
            tz::update();
            window.update();
        }
        
    }
    tz::terminate();
    return 0;
}