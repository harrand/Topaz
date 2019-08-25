//
// Created by Harrand on 25/08/2019.
//

#ifndef TOPAZ_FONT_HPP
#define TOPAZ_FONT_HPP
#include "ext/freetype/include/ft2build.h"
#include FT_FREETYPE_H
#include <string>
#include "graphics/image.hpp"

namespace tz::graphics::detail
{
    extern FT_Library freetype_library;
    extern bool freetype_initialized;
}

/**
* Used to render text. Texture has a constructor taking a Font as a parameter. Use this to achieve font-rendering.
*/
class Font
{
public:
    /**
     * Construct a font from an existing font-file.
     * @param font_path - Path to the supported font-file.
     * @param pixel_height - Height of the glyphs, in pixels.
     */
    Font(const std::string& font_path, int pixel_height);
    /**
     * Construct a Font from another Font's font-file.
     * @param copy - The Font whose font-file should be used.
     */
    Font(const Font& copy);
    /**
     * Construct a Font, taking control of an existing Font's font-data.
     * @param move - The Font whose font-data should be taken.
     */
    Font(Font&& move);
    /**
     * Safely dispose of font-data.
     */
    ~Font();
    /**
     * Copy-and-swap idiom copy-assignment.
     * @param rhs - Font whose data should be copied
     * @return - The resultant font
     */
    Font& operator=(Font rhs);
    /**
     * Trivial move-assignment.
     * @param rhs - Font whose data members should be taken.
     * @return - The resultant font.
     */
    Font& operator=(Font&& rhs);
    /**
     * Get the height of glyphs, in pixels.
     * @return - Height of glyphs, in pixels.
     */
    int get_pixel_height() const;
    /**
     * Retrieve the filename of the file used to load this Font.
     * @return - Filename of the source font.
     */
    const std::string& get_path() const;
    std::vector<Image> render_bitmap(const std::string& text) const;
private:
    /// Path to the font-file containing font-data.
    std::string font_path;
    /// Height of glyphs, in pixels.
    int pixel_height;
    /// FreeType graphical font-data.
    FT_Face font_handle;
};



#endif //TOPAZ_FONT_HPP
