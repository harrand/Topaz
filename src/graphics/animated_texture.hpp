//
// Created by Harrand on 01/09/2018.
//

#ifndef TOPAZ_ANIMATED_TEXTURE_HPP
#define TOPAZ_ANIMATED_TEXTURE_HPP
#include "graphics/texture.hpp"
#include "utility/time.hpp"

class PolyFrameTexture
{
public:
    using FrameMap = std::map<std::size_t, Texture>;
    //PolyFrameTexture(const std::string& animated_texture_filename);
    PolyFrameTexture();
    PolyFrameTexture(FrameMap frames);

    class iterator
    {
    public:
        iterator(PolyFrameTexture& animation, std::size_t index);
        Texture& operator*() const;
        bool operator!=(const iterator& rhs) const;
        iterator& operator++();
        iterator& operator--();
    private:
        PolyFrameTexture& animation;
        std::size_t index;
    };

    class const_iterator
    {
    public:
        const_iterator(const PolyFrameTexture& animation, std::size_t index);
        const Texture& operator*() const;
        bool operator!=(const const_iterator& rhs) const;
        const_iterator& operator++();
        const_iterator& operator--();
    private:
        const PolyFrameTexture& animation;
        std::size_t index;
    };
    iterator begin();
    const_iterator cbegin() const;
    iterator end();
    const_iterator cend() const;

    const Texture* get_frame(std::size_t frame) const;
    void set_frame(std::size_t frame, Texture texture);
    std::size_t get_number_of_frames() const;
private:
    std::map<std::size_t, Texture> frames;
};

class AnimatedTexture : public PolyFrameTexture, public FrameScheduler
{
public:
    AnimatedTexture(PolyFrameTexture::FrameMap frames, unsigned int fps);
    const Texture& get_frame_texture() const;
private:
    using FrameScheduler::get_number_of_frames;
    using FrameScheduler::set_number_of_frames;
    using FrameScheduler::get_current_frame;
};


#endif //TOPAZ_ANIMATED_TEXTURE_HPP
