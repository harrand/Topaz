#include "../scene.hpp"
#include "../topaz.hpp"
#include "../window.hpp"
#include "lib/test_util.hpp"

void test();

int main()
{
    tz::initialise();
    std::cerr << "Scene Test Results: " << std::endl;
    try
    {
        test();
        std::cerr << "PASS\n";
    }catch(const TestFailureException& fail)
    {
        std::cerr << "FAIL: " << fail.what() << "\n";
    }
    tz::terminate();
    return 0;
}

void test()
{
    // only have a window because we need an opengl context, which only a window provides. it'll be invisible anyway.
    Window wnd("Hello there.", {}, {});
    wnd.set_minimised(true);
    // create an empty scene.
    Scene scene;
    // ensure there are no objects in the scene.
    tz::assert::equal(scene.get_objects().size(), 0);
    // now emplace some pointless object.
    scene.emplace<StaticObject>(Transform{{}, {}, {}}, Asset{{}, {}});
    // now we expect the scene to have a size of 1.
    tz::assert::equal(scene.get_objects().size(), 1);
    // now add another pointless object.
    scene.add_object(StaticObject{Transform{{}, {}, {}}, Asset{{}, {}}});
    tz::assert::equal(scene.get_objects().size(), 2);
}