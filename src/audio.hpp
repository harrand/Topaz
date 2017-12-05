#ifndef AUDIO_HPP
#define AUDIO_HPP
#include <string>
#include "SDL_mixer.h"
#include "camera.hpp"
#include "utility.hpp"

/*
	Initialise and terminate tz audio module. This must be done appropriately to use any of the audio functionality.
*/
namespace tz
{
	namespace audio
	{
		// No audio works until this is executed. tz::initialise will invoke this automatically.
		void initialise();
		// Memory droplets will remain until this is executed. Audio will not work after this is invoked. tz::terminate will invoke this automatically.
		void terminate();
	}
}

/*
	Playable audio file. Use this for short audio segments like sound effects.
*/
class AudioClip
{
public:
	AudioClip(std::string filename);
	AudioClip(const AudioClip& copy);
	AudioClip(AudioClip&& move);
	virtual ~AudioClip();
	AudioClip& operator=(const AudioClip& rhs) = delete;
	
	void play();
	int get_channel() const;
	const std::string& get_file_name() const;
private:
	int channel;
	const std::string filename;
	Mix_Chunk* audio_handle;
};

/*
	Playable audio file, but from a position in 3D space. Same properties as AudioClip.
*/
class AudioSource: public AudioClip
{
public:
	AudioSource(std::string filename);
	AudioSource(const AudioSource& copy) = default;
	AudioSource(AudioSource&& move) = default;
	~AudioSource() = default;
	AudioSource& operator=(const AudioSource& rhs) = default;
	
	// Should be invoked whenever the camera rotates or moves or the AudioSource position is changed.
	void update(const Vector3F& source_position, const Camera& relative_to);
};

/*
	Playable audio file. Use this for longer audio segments such as background music.
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
	// Play should be invoked only once and not to un-pause music.
	void play(bool priority = true) const;
	// Pause/Resume the music.
	void set_paused(bool pause = true);
private:
	std::string filename;
	bool paused;
	Mix_Music* audio_handle;
};

#endif