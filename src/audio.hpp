#ifndef AUDIO_HPP
#define AUDIO_HPP
#include <string>
#include "SDL_mixer.h"
#include "camera.hpp"
#include "utility.hpp"

namespace tz
{
	namespace audio
	{
		void initialise();
		void terminate();
	}
}

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
	Mix_Chunk* get_audio_handle() const;
private:
	int channel;
	const std::string filename;
	Mix_Chunk* audio_handle;
};

class AudioSource: public AudioClip
{
public:
	AudioSource(std::string filename, Vector3F position);
	AudioSource(const AudioSource& copy) = default;
	AudioSource(AudioSource&& move) = default;
	~AudioSource() = default;
	AudioSource& operator=(const AudioSource& rhs) = default;
	
	void update(const Camera& relative_to);
	
	Vector3F position;
};

class AudioMusic
{
public:
	AudioMusic(std::string filename);
	AudioMusic(const AudioMusic& copy);
	AudioMusic(AudioMusic&& move);
	~AudioMusic();
	AudioMusic& operator=(const AudioMusic& rhs) = delete;
	
	const std::string& get_file_name() const;
	Mix_Music* get_audio_handle() const;
	void play(bool priority = true) const;
	void set_paused(bool pause = true);
	void toggle();
private:
	std::string filename;
	bool paused;
	Mix_Music* audio_handle;
};

#endif