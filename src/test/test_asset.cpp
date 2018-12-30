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
    // it's raw.
    tz::assert::that(asset_buffer.find_mesh(raw) == nullptr);
    asset_buffer.emplace_mesh(raw, raw);
    tz::assert::that(asset_buffer.find_mesh(raw) != nullptr);
    tz::assert::that(asset_buffer.find_texture(raw) == nullptr);
    asset_buffer.emplace_texture(raw, raw);
    tz::assert::that(asset_buffer.find_texture(raw) != nullptr);
    tz::assert::that(asset_buffer.find_normal_map(raw) == nullptr);
    asset_buffer.emplace_normalmap(raw, raw);
    tz::assert::that(asset_buffer.find_normal_map(raw) != nullptr);
    tz::assert::that(asset_buffer.find_parallax_map(raw) == nullptr);
    asset_buffer.emplace_parallaxmap(raw, raw);
    tz::assert::that(asset_buffer.find_parallax_map(raw) != nullptr);
    tz::assert::that(asset_buffer.find_displacement_map(raw) == nullptr);
    asset_buffer.emplace_displacementmap(raw, raw);
    tz::assert::that(asset_buffer.find_displacement_map(raw) != nullptr);
}