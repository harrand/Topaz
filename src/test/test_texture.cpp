#include "graphics/gui/button.hpp"
#include "core/topaz.hpp"
#include "core/window.hpp"
#include "lib/test_util.hpp"

void test();

int main()
{
    tz::initialise();
    std::cerr << "Texture Test Results: " << std::endl;
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
    PixelRGBA red_pixel{255, 0, 0, 255};
    PixelRGBA blue_pixel{0, 0, 255, 255};
    Bitmap<PixelRGBA> example_bitmap{std::vector<PixelRGBA>{red_pixel, blue_pixel, red_pixel, blue_pixel}, 2, 2};

    Window wnd("Test Texture - Requires Manual Input", 0, 30, 800, 600);
    Shader gui_shader("../../../src/shaders/Gui");
    Font font("../../../res/runtime/fonts/CaviarDreams.ttf", 36);
    Texture example_texture{example_bitmap};
    Texture example_copy{example_texture};
    Texture example_copy_copy{example_copy};
    Texture example_texture2{"../../../res/runtime/textures/wood.jpg"};
    Texture example_copy2{example_copy};
    example_copy2 = example_texture2;
    Texture example_copy2_copy{example_copy2};
    // bottom row
    wnd.emplace_child<Panel>(Vector2I{}, Vector2I{100, 100}, &example_texture);
    wnd.emplace_child<Panel>(Vector2I{300, 0}, Vector2I{100, 100}, &example_copy);
    wnd.emplace_child<Panel>(Vector2I{600, 0}, Vector2I{100, 100}, &example_copy_copy);
    // top row
    wnd.emplace_child<Panel>(Vector2I{0, 500}, Vector2I{100, 100}, &example_texture2);
    wnd.emplace_child<Panel>(Vector2I{300, 500}, Vector2I{100, 100}, &example_copy2);
    wnd.emplace_child<Panel>(Vector2I{600, 500}, Vector2I{100, 100}, &example_copy2_copy);
    bool worked = false;
    Button& confirm = wnd.emplace_child<Button>(Vector2I{0, 200}, Vector2I{200, 20}, font, Vector3F{}, "Click me if the images are the same.", Vector3F{0.2f, 0.2f, 0.2f}, Vector3F{0.6f, 0.6f, 0.6f});
    confirm.set_callback([&worked](){worked = true;});

    while(!wnd.is_close_requested())
    {
        wnd.update(gui_shader);
        if(worked)
            break;
    }
    tz::assert::that(worked);
}