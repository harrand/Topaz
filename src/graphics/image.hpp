//
// Created by Harrand on 24/08/2019.
//

#ifndef TOPAZ_IMAGE_HPP
#define TOPAZ_IMAGE_HPP
#include "utility/memory.hpp"
#include "assimp/texture.h"
#include <string>
#include <cstddef>
#include <vector>

/**
 * Wrapper for an image. Format is always RGBA8888.
 */
class Image
{
public:
    Image(std::string path);
    Image(const aiTexture* assimp_texture);
    Image(std::vector<std::byte> image_data, unsigned int width, unsigned int height);
    const BytePool data() const;
    BytePool data();
    Vector2UI get_dimensions() const;
    unsigned int get_width() const;
    unsigned int get_height() const;
    std::size_t get_size() const;
    bool has_filename() const;
    const std::string* get_filename() const;
private:
    std::optional<std::string> path;
    std::vector<std::byte> image_data;
    unsigned int width, height;
};


#endif //TOPAZ_IMAGE_HPP
