//
// Created by Harrand on 25/12/2019.
//

#include "test_framework.hpp"
#include "core/core.hpp"
#include "input/system_input.hpp"
#include <string>
#include <cstring>
 
tz::test::Case set_and_get()
{
    tz::test::Case test_case("tz::input Clipboard Tests");
    // Make a copy of the current state of the clipboard (So we can restore it without screwing up the user's clipboard).
    const char* cache = tz::input::get_clipboard_data();
    if(cache == nullptr)
        cache = "";
    std::string clipboard_cache = {cache};

    // We're free to use this forever. This will also probably be visible in the binary which is pretty funny
    const char* perm_data = "follow @Harrandev on twitter";
    tz::input::set_clipboard_data(perm_data);
    topaz_expect(test_case, std::strcmp(perm_data, tz::input::get_clipboard_data()) == 0, "tz::input failed to set clipboard data properly (\"", perm_data, "\" and \"", tz::input::get_clipboard_data(), "\"");

    // Re-seat the inital clipboard data so to not disrupt the user's workflow.
    tz::input::set_clipboard_data(clipboard_cache.c_str());
    topaz_expect(test_case, clipboard_cache == tz::input::get_clipboard_data(), "tz::input failed to preserve initial clipboard data.");
    return test_case;
}

int main()
{
    tz::test::Unit clip;

    // Stuff in here requires topaz to be initialised.
    {
        tz::core::initialise("Clipboard Tests");

        clip.add(set_and_get());

        tz::core::terminate();
    }

    return clip.result();
}