#ifndef AUDIO_HPP
#define AUDIO_HPP
#include <string>
#include "SDL2/SDL_mixer.h"
#include "camera.hpp"
#include "utility.hpp"

/**
*	Playable audio file. Use this for short audio segments like sound effects.
*/
class AudioClip
{
public:
	/**
	* Load AudioClip from existing file (must be wavefront audio .wav)
	*/
	AudioClip(std::string filename);
	/**
	* Construct AudioClip using the filename of copy.
	*/
	AudioClip(const AudioClip& copy);
	/**
	* Construct AudioClip using the same chunk as move. Also copies move's filename.
	*/
	AudioClip(AudioClip&& move);
	/**
	* Deallocate memory from the SDL_Mixer functionality.
	*/
	virtual ~AudioClip();
	AudioClip& operator=(const AudioClip& rhs) = delete;
	
	/**
	* Plays the audio. 
	* The audio will play until either the destructor is called or the audio is finished; whichever takes place first.
	* Note: Invoking tz::audio::play_async on an instance of AudioClip will extend the lifetime of the instance such that the audio clip is guaranteed to be fully played.
	*/
	void play();
	int get_channel() const;
	Uint32 get_audio_length() const;
	const std::string& get_file_name() const;
private:
	int channel;
	const std::string filename;
	Mix_Chunk* audio_handle;
};

/**
*	Playable audio file, but from a position in 3D space. Same properties as AudioClip.
*/
class AudioSource: public AudioClip
{
public:
	AudioSource(std::string filename);
	AudioSource(const AudioSource& copy) = default;
	AudioSource(AudioSource&& move) = default;
	~AudioSource() = default;
	AudioSource& operator=(const AudioSource& rhs) = delete;
	
	/**
	* Should be invoked whenever the camera rotates or moves or the AudioSource position is changed.
	*/
	void update(const Vector3F& source_position, const Camera& relative_to) const;
};

/**
*	Playable audio file. Use this for longer audio segments such as background music.
*/
class AudioMusic
{
public:
	AudioMusic(std::string filename);
	AudioMusic(const AudioMusic& copy);
	AudioMusic(AudioMusic&& move);
	~AudioMusic();
	AudioMusic& operator=(const AudioMusic& rhs) = delete;
	
	const std::string& get_file_name() const;
	bool is_paused() const;
	/**
	* Play should be invoked only once and not to un-pause music.
	*/
	void play(bool priority = true) const;
	/**
	* Pause/Resume the music.
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
		* Play existing Audio asynchronously (Anything that has a play() function). Has much reduced overhead for an r-value reference.
		*/
		template<typename Audio>
		void play_async(Audio&& clip);
	}
}

#include "audio.inl"
#endif