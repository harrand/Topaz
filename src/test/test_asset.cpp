#define UNIT_TEST "Asset"
#include "lib/test_util.hpp"
#include "core/window.hpp"
#include "graphics/asset.hpp"


void test()
{
    // only have a window because we need an opengl context, which only a window provides. it'll be invisible anyway.
    Window wnd("Hello there.", {}, {});
    wnd.set_minimised(true);
    AssetBuffer asset_buffer{{}, {}};
    std::string raw = "raw";
    std::string raw_normal = "raw normal";
    std::string raw_parallax = "raw parallax";
    std::string raw_displacement = "raw displacement";
    // it's raw.
    tz::assert::that(asset_buffer.find<Mesh>(raw) == nullptr);
    asset_buffer.emplace_mesh(raw, raw);
    tz::assert::that(asset_buffer.find<Mesh>(raw) != nullptr);
    tz::assert::that(asset_buffer.find<Texture>(raw) == nullptr);
    asset_buffer.emplace<Texture>(raw, raw);
    tz::assert::that(asset_buffer.find<Texture>(raw) != nullptr);
    asset_buffer.emplace<NormalMap>(raw_normal, raw);
    tz::assert::that(asset_buffer.find<NormalMap>(raw_normal) != nullptr);
    tz::assert::that(asset_buffer.find<ParallaxMap>(raw_parallax) == nullptr);
    asset_buffer.emplace<ParallaxMap>(raw_parallax, raw);
    tz::assert::that(asset_buffer.find<ParallaxMap>(raw_parallax) != nullptr);
    tz::assert::that(asset_buffer.find<DisplacementMap>(raw_displacement) == nullptr);
    asset_buffer.emplace<DisplacementMap>(raw_displacement, raw);
    tz::assert::that(asset_buffer.find<DisplacementMap>(raw_displacement) != nullptr);
}