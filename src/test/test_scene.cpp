#include "core/scene.hpp"
#include "core/topaz.hpp"
#include "core/window.hpp"
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

void test_constructor()
{
    // create an empty scene.
    Scene scene;
    // ensure there are no objects in the scene.
    tz::assert::equal<int, int>(scene.get_objects().size(), 0);
    StaticObject test_object{Transform{{}, {}, {}}, Asset{{}, {}}};
    scene = Scene({test_object});
    // ensure this new scene has its initialisation list (for stack object) correctly done.
    tz::assert::equal<int, int>(scene.get_objects().size(), 1);
    std::vector<std::unique_ptr<StaticObject>> heap_objs;
    heap_objs.push_back(std::make_unique<StaticObject>(Transform{{}, {}, {}}, Asset{{}, {}}));
    // give it a heap object too now.
    scene = Scene({test_object}, std::move(heap_objs));
    tz::assert::equal<int, int>(scene.get_objects().size(), 2);
}

void test_emplacement()
{
    // create an empty scene.
    Scene scene;
    // ensure there are no objects in the scene.
    tz::assert::equal<int, int>(scene.get_objects().size(), 0);
    // now emplace some pointless object.
    scene.emplace<StaticObject>(Transform{{}, {}, {}}, Asset{{}, {}});
    // now we expect the scene to have a size of 1.
    tz::assert::equal<int, int>(scene.get_objects().size(), 1);
    // now add another pointless object.
    scene.add_object(StaticObject{Transform{{}, {}, {}}, Asset{{}, {}}});
    tz::assert::equal<int, int>(scene.get_objects().size(), 2);
}

void test()
{
    // only have a window because we need an opengl context, which only a window provides. it'll be invisible anyway.
    Window wnd("Hello there.", {}, {});
    wnd.set_minimised(true);
    test_constructor();
    test_emplacement();
}