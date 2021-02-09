#include "core/core.hpp"

int main()
{
    tz::core::initialise("Topaz Window Demo");
    {
        tz::core::IWindow& window = tz::core::get().window();

        while(!window.is_close_requested())
        {
            // Do stuff.
            tz::core::update();
            window.update();
        }
        
    }
    tz::core::terminate();
    return 0;
}