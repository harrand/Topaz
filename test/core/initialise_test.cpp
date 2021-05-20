#include "core/tz.hpp"

int main()
{
    tz::initialise({"tz_initialise_test", tz::EngineInfo::Version{1, 0, 0}, tz::info()}, tz::ApplicationType::Headless);
    {
        // Nothing
    }
    tz::terminate();
}