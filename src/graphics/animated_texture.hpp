//
// Created by Harrand on 01/09/2018.
//

#ifndef TOPAZ_ANIMATED_TEXTURE_HPP
#define TOPAZ_ANIMATED_TEXTURE_HPP
#include "graphics/texture.hpp"
#include "utility/time.hpp"

/**
 * A container for a collection of frames. Each frame is represented by a Texture object.
 */
class PolyFrameTexture
{
public:
    /// Container of Textures constituting frames.
    using FrameMap = std::map<std::size_t, Texture>;
    /**
     * Construct an empty PolyFrameTexture.
     */
    PolyFrameTexture();
    /**
     * Construct a PolyFrameTexture from a given container of frames.
     * @param frames
     */
    PolyFrameTexture(FrameMap frames);

    using iterator = PolyFrameTexture::FrameMap::iterator;
    using const_iterator = PolyFrameTexture::FrameMap::const_iterator;

    /// Range-based for-loop support.
    iterator begin();
    /// Range-based for-loop support.
    const_iterator cbegin() const;
    /// Range-based for-loop support.
    iterator end();
    /// Range-based for-loop support.
    const_iterator cend() const;

    /**
     * Get the Texture representing the frame at the given frame number.
     * @param frame - The frame number to retrieve
     * @return - The frame, represented as a Texture
     */
    const Texture* get_frame(std::size_t frame) const;
    /**
     * Set the given Texture to be the frame at the given frame number.
     * @param frame - The number of the frame to replace
     * @param texture - The Texture to be used as the new frame
     */
    void set_frame(std::size_t frame, Texture texture);
    /**
     * Get the number of frames in this PolyFrameTexture.
     * @return - Number of frames
     */
    std::size_t get_number_of_frames() const;
private:
    /// Stores all of the frame data.
    PolyFrameTexture::FrameMap frames;
};

/**
 * An object representing an animated Texture.
 */
class AnimatedTexture : public PolyFrameTexture, public FrameScheduler
{
public:
    /**
     * Construct an AnimatedTexture from a container of frames and a fps through which to run at.
     * @param frames - The container of frame textures
     * @param fps - The number of frames to be played per second
     */
    AnimatedTexture(PolyFrameTexture::FrameMap frames, unsigned int fps);
    /**
     * Get the Texture representing the current frame.
     * @return - Texture of the current frame
     */
    const Texture& get_frame_texture() const;
private:
    using FrameScheduler::set_number_of_frames;
};


#endif //TOPAZ_ANIMATED_TEXTURE_HPP
