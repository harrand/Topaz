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
	~AudioClip();
	AudioClip& operator=(const AudioClip& rhs) = delete;
	
	void play();
	int getChannel() const;
	const std::string& getFileName() const;
	Mix_Chunk* getAudioHandle() const;
private:
	int channel;
	std::string filename;
	Mix_Chunk* audio_handle;
};

class AudioSource: public AudioClip
{
public:
	AudioSource(std::string filename, Vector3F position);
	void update(const Camera& relative_to);
	const Vector3F& getPosition() const;
	Vector3F& getPositionR();
private:
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
	
	const std::string& getFileName() const;
	Mix_Music* getAudioHandle() const;
	void play(bool priority = true) const;
	void setPaused(bool pause = true);
	void togglePaused();
private:
	std::string filename;
	bool paused;
	Mix_Music* audio_handle;
};

#endif