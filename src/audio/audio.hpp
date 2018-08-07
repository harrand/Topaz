#ifndef AUDIO_HPP
#define AUDIO_HPP
#include <string>
#include "SDL2/SDL_mixer.h"
#include "graphics/camera.hpp"

namespace tz::audio
{
    constexpr int unused_channel = -1;
}

/**
*	Playable audio file. Use this for short audio segments like sound effects.
*/
class AudioClip
{
public:
	/**
	 * Construct an audioclip from an existing file with supported format.
	 * @param filename - Path of the filename.
	 */
	AudioClip(std::string filename);
	/**
	 * Construct an audioclip from an existing AudioClip's source file.
	 * @param copy - The existing AudioClip whose source file should be used.
	 */
	AudioClip(const AudioClip& copy);
	/**
	 * Construct an audioclip, taking control of an existing AudioClip's audio data.
	 * @param move - The existing AudioClip whose audio data should be taken from.
	 */
	AudioClip(AudioClip&& move);
	/**
	 * Dispose of audio data sensibly.
	 */
	virtual ~AudioClip();
	/// Rule of Five
	AudioClip& operator=(AudioClip&& rhs);
	/// Rule of Five
	AudioClip& operator=(AudioClip rhs);
	/**
	* Plays the audio. 
	* The audio will play until either the destructor is called or the audio is finished; whichever takes place first.
	* Note: Invoking tz::audio::play_async on an instance of AudioClip will extend the lifetime of the instance such that the audio clip is guaranteed to be fully played.
	*/
	void play();
	/**
	 * Get the channel that this AudioClip is currently playing on.
	 * @return - Channel that the clip is playing on
	 */
	int get_channel() const;
	/**
	 * Estimate the length of the AudioClip, in milliseconds.
	 * @return - Length of the clip, in milliseconds.
	 */
	Uint32 get_audio_length() const;
	/**
	 * Retrieve the filename of the file used to load this AudioClip.
	 * @return - Filename of the source audio.
	 */
	const std::string& get_file_name() const;
protected:
    static void swap(AudioClip& lhs, AudioClip& rhs);
    /// Channel that the AudioClip is currently playing on.
	int channel;
	/// Path to the file used to load this AudioClip's data.
	std::string filename;
	/// SDL_Mixer audio data handle.
	Mix_Chunk* audio_handle;
};

/**
*	Playable audio file, but from a position in 3-dimensional space. Same properties as AudioClip.
*/
class AudioSource: public AudioClip
{
public:
	/**
	 * Construct an audioclip from an existing file with supported format.
	 * @param filename - Path of the filename.
	 */
	AudioSource(std::string filename);
	AudioSource(const AudioSource& copy);
	AudioSource(AudioSource&& move);
	/// Destructor is same as AudioClip, so not included here. I will not write an empty one just to satisfy the Ro5.
	AudioSource& operator=(AudioSource&& rhs);
	AudioSource& operator=(AudioSource rhs);

	/**
	 * Invoke whenever the orientation of the camera changes.
	 * This can also be invoked to change where the position of the audio-source should be.
	 * @param source_position - The position in 3-dimensional space of the audio's source.
	 * @param relative_to - The camera representing the listener's properties.
	 */
	void update(const Vector3F& source_position, const Camera& relative_to) const;
};

/**
*	Playable audio file. Use this for longer audio segments such as background music.
*/
class AudioMusic
{
public:
	/**
	 * Construct an audioclip from an existing file with supported format.
	 * @param filename - Path of the filename.
	 */
	AudioMusic(std::string filename);
	/**
	 * Construct an audioclip from an existing AudioClip's source file.
	 * @param copy - The existing AudioClip whose source file should be used.
	 */
	AudioMusic(const AudioMusic& copy);
	/**
	 * Construct an audioclip, taking control of an existing AudioClip's audio data.
	 * @param move - The existing AudioClip whose audio data should be taken from.
	 */
	AudioMusic(AudioMusic&& move);
	/**
	 * Dispose of audio data sensibly.
	 */
	~AudioMusic();
	AudioMusic& operator=(AudioMusic&& rhs);
	AudioMusic& operator=(AudioMusic rhs);
	/**
	 * Retrieve the filename of the file used to load this AudioClip.
	 * @return - Filename of the source audio.
	 */
	const std::string& get_file_name() const;
	/**
	 * Query whether this AudioMusic is paused or not.
	 * @return - True if the music is paused. False otherwise.
	 */
	bool is_paused() const;
	/**
	 * Play the AudioMusic, specifying priority.
	 * @param priority - Whether the AudioMusic should stop any other AudioMusic from playing or not.
	 */
	void play(bool priority = true) const;
	/**
	 * Pause/resume the music.
	 * @param pause - Whether to pause or resume the music.
	 */
	void set_paused(bool pause = true);
protected:
    void swap(AudioMusic& lhs, AudioMusic& rhs);
	/// Path to the file used to load this AudioClip's data.
	std::string filename;
	/// Controls whether the music is paused or playing.
	bool paused;
	/// SDL_Mixer audio data handle.
	Mix_Music* audio_handle;
};

/**
*	Initialise and terminate tz audio module. This must be done appropriately to use any of the audio functionality.
*/
namespace tz
{
	namespace audio
	{
		/**
		* No audio works until this is executed. tz::initialise will invoke this automatically.
		*/
		void initialise(bool print_progress = false);
		/**
		* Memory droplets will remain until this is executed. Audio will not work after this is invoked. tz::terminate will invoke this automatically.
		*/
		void terminate(bool print_progress = false);
		/**
		 * Play any type of supported audio, asynchronously.
		 * Returns as soon as the audio starts playing.
		 * @tparam Audio - The type of audio to play.
		 * @param clip - The audio which should be played.
		 */
		template<typename Audio>
		void play_async(Audio&& clip);
	}
}

#include "audio.inl"
#endif