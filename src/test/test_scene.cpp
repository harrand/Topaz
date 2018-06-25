#include "../scene.hpp"
#include "../topaz.hpp"
#include "../window.hpp"

bool test();

int main()
{
    tz::initialise();
    std::cerr << "Test Results: " << std::endl;
    std::cerr << std::boolalpha << test() << std::endl;
    tz::terminate();
    return 0;
}

bool test()
{
    Window wnd("Hello there.", {}, {});
    Scene scene;
    if(scene.get_objects().size() != 0)
        return false;
    scene.emplace<StaticObject>(Transform{{}, {}, {}}, Asset{{}, {}});
    if(scene.get_objects().size() != 1)
        return false;
    return true;
}