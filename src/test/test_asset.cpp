#include "../topaz.hpp"
#include "../window.hpp"
#include "../asset.hpp"
#include "lib/test_util.hpp"

void test();

int main()
{
    tz::initialise();
    std::cerr << "Asset Test Results: " << std::endl;
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

void test_textures()
{
    AssetBuffer asset_buffer{{}, {}};
    tz::assert::that(asset_buffer.find_texture("i do not exist") == nullptr);
    asset_buffer.emplace_texture("empty ass texture");
    tz::assert::that(asset_buffer.find_texture("empty ass texture") != nullptr);
}

void test()
{
    // only have a window because we need an opengl context, which only a window provides. it'll be invisible anyway.
    Window wnd("Hello there.", {}, {});
    wnd.set_minimised(true);
    test_textures();
}
