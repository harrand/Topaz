//
// Created by Harrand on 01/09/2018.
//

#include "animated_texture.hpp"

PolyFrameTexture::PolyFrameTexture(): frames(){}
PolyFrameTexture::PolyFrameTexture(PolyFrameTexture::FrameMap frames)
{
    for(std::size_t i = 0; i < frames.size(); i++)
        this->set_frame(i, frames.at(i));
}

PolyFrameTexture::iterator PolyFrameTexture::begin()
{
    return this->frames.begin();
}

PolyFrameTexture::const_iterator PolyFrameTexture::cbegin() const
{
    return this->frames.cbegin();
}

PolyFrameTexture::iterator PolyFrameTexture::end()
{
    return this->frames.end();
}

PolyFrameTexture::const_iterator PolyFrameTexture::cend() const
{
    return this->frames.cend();
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
    if(this->frames.size() == 0 || frame >= this->frames.size())
    {
        // current frames cant fit our new addition. find out how many additions we need:
        std::size_t required_additions = frame + 1 - this->frames.size();
        // then add that number of additions.
        for(std::size_t i = 0; i < required_additions; i++)
            this->frames.insert({frame, std::move(texture)});
    }
    else
    {
        // current frames can fit our new addition, just pick and replace.
        this->frames.at(frame) = texture;
    }
    /*
    if(frame - 1 <= this->frames.size())
    {
        if(this->frames.find(frame) != this->frames.end())
        {
            std::cout << "option a.\n";
            this->frames[frame] = texture;
        }
        else
        {
            std::cout << "option b.\n";
            this->frames.insert({frame, texture});
        }
    }
    else
    {
        std::cout << "option c.\n";
        // we need to expand frames out. we wont interpolate, so just make copies of the texture for each frame until the new last
        for(auto i = this->frames.size(); i < (frame - 1); i++)
            this->set_frame(i, texture);
        std::cout << "okay.\n";
    }
     */
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