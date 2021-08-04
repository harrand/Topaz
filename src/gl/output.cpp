#include "gl/output.hpp"

namespace tz::gl
{
    void TextureOutput::add_colour_output(tz::gl::TextureComponent* texture_component)
    {
        this->colour_outputs.push_back(texture_component);
    }

    void TextureOutput::set_depth_output(tz::gl::TextureComponent* texture_component)
    {
        this->depth_attachment = texture_component;
    }
}