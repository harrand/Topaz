#ifndef AUDIO_HPP
#define AUDIO_HPP
#include <string>
#include "SDL2/SDL_mixer.h"
#include "../camera.hpp"
#include "../utility.hpp"

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
	AudioClip(const AudioClip& copy);
	AudioClip(AudioClip&& move);
	virtual ~AudioClip();
	AudioClip& operator=(const AudioClip& rhs) = delete;
	
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
private:
	int channel;
	const std::string filename;
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
	AudioSource(const AudioSource& copy) = default;
	AudioSource(AudioSource&& move) = default;
	~AudioSource() = default;
	AudioSource& operator=(const AudioSource& rhs) = delete;

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
	AudioMusic(const AudioMusic& copy);
	AudioMusic(AudioMusic&& move);
	~AudioMusic();
	AudioMusic& operator=(const AudioMusic& rhs) = delete;

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
private:
	std::string filename;
	bool paused;
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
		void initialise();
		/**
		* Memory droplets will remain until this is executed. Audio will not work after this is invoked. tz::terminate will invoke this automatically.
		*/
		void terminate();
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