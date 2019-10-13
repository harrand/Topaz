#define UNIT_TEST "Scene"
#include "lib/test_util.hpp"
#include "core/scene.hpp"

void test_constructor()
{
    // create an empty scene.
    Scene scene;
    // ensure there are no objects in the scene.
    tz::assert::equal<int, int>(scene.get_static_objects().size(), 0);
    StaticObject test_object{Transform{{}, {}, {}}, Asset{{}, {}}};
    scene.emplace_object(test_object);
    // ensure this new scene has its initialisation list (for stack object) correctly done.
    tz::assert::equal<int, int>(scene.get_static_objects().size(), 1);
}

void test_emplacement()
{
    // create an empty scene.
    Scene scene;
    // ensure there are no objects in the scene.
    tz::assert::equal<int, int>(scene.get_static_objects().size(), 0);
    // now emplace some pointless object.
    scene.emplace<StaticObject>(Transform{{}, {}, {}}, Asset{{}, {}});
    // now we expect the scene to have a size of 1.
    tz::assert::equal<int, int>(scene.get_static_objects().size(), 1);
    // now add another pointless object.
    scene.emplace_object(Transform{{}, {}, {}}, Asset{{}, {}});
    tz::assert::equal<int, int>(scene.get_static_objects().size(), 2);
}

void test()
{
    // only have a window because we need an opengl context, which only a window provides. it'll be invisible anyway.
    Window wnd("Hello there.", {}, {});
    wnd.set_minimised(true);
    test_constructor();
    test_emplacement();
}