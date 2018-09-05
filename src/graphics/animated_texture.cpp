//
// Created by Harrand on 01/09/2018.
//

#include "animated_texture.hpp"

//PolyFrameTexture::PolyFrameTexture(const std::string &animated_texture_filename){}
PolyFrameTexture::PolyFrameTexture(): frames(){}
PolyFrameTexture::PolyFrameTexture(PolyFrameTexture::FrameMap frames): frames(frames){}

PolyFrameTexture::iterator::iterator(PolyFrameTexture& animation, std::size_t index): animation(animation), index(index){}

Texture& PolyFrameTexture::iterator::operator*() const
{
    return this->animation.frames.at(this->index);
}

bool PolyFrameTexture::iterator::operator!=(const iterator &rhs) const
{
    return &this->animation != &rhs.animation || this->index != rhs.index;
}

PolyFrameTexture::iterator& PolyFrameTexture::iterator::operator++()
{
    this->index++;
    return *this;
}

PolyFrameTexture::iterator& PolyFrameTexture::iterator::operator--()
{
    this->index--;
    return *this;
}

PolyFrameTexture::const_iterator::const_iterator(const PolyFrameTexture& animation, std::size_t index): animation(animation), index(index){}

const Texture& PolyFrameTexture::const_iterator::operator*() const
{
    return this->animation.frames.at(this->index);
}

bool PolyFrameTexture::const_iterator::operator!=(const const_iterator &rhs) const
{
    return &this->animation != &rhs.animation || this->index != rhs.index;
}

PolyFrameTexture::const_iterator& PolyFrameTexture::const_iterator::operator++()
{
    this->index++;
    return *this;
}

PolyFrameTexture::const_iterator& PolyFrameTexture::const_iterator::operator--()
{
    this->index--;
    return *this;
}

PolyFrameTexture::iterator PolyFrameTexture::begin()
{
    return {*this, 0};
}

PolyFrameTexture::const_iterator PolyFrameTexture::cbegin() const
{
    return {*this, 0};
}

PolyFrameTexture::iterator PolyFrameTexture::end()
{
    return {*this, this->frames.size() - 1};
}

PolyFrameTexture::const_iterator PolyFrameTexture::cend() const
{
    return {*this, this->frames.size() - 1};
}

const Texture* PolyFrameTexture::get_frame(std::size_t frame) const
{
    try
    {
        return &this->frames.at(frame);
    }
    catch(const std::out_of_range&)
    {
        return nullptr;
    }
}

void PolyFrameTexture::set_frame(std::size_t frame, Texture texture)
{
    if(frame - 1 <= this->frames.size())
    {
        if(this->frames.find(frame) != this->frames.end())
            this->frames[frame] = texture;
        else
            this->frames.insert({frame, texture});
    }
    else
    {
        // we need to expand frames out. we wont interpolate, so just make copies of the texture for each frame until the new last
        for(auto i = this->frames.size(); i < (frame - 1); i++)
            this->set_frame(i, texture);
    }
}

std::size_t PolyFrameTexture::get_number_of_frames() const
{
    return this->frames.size();
}

AnimatedTexture::AnimatedTexture(PolyFrameTexture::FrameMap frames, unsigned int fps): PolyFrameTexture(frames), FrameScheduler(PolyFrameTexture::get_number_of_frames(), fps){}

const Texture& AnimatedTexture::get_frame_texture() const
{
    return *this->get_frame(this->get_current_frame());
}