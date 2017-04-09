#ifndef AUDIO_HPP
#define AUDIO_HPP
#include <string>
#include "SDL_mixer.h"

class AudioClip
{
public:
	AudioClip(std::string filename);
	~AudioClip();
	void play();
	int getChannel() const;
private:
	int channel;
	const std::string filename;
	Mix_Chunk* audioHandle;
};

class AudioMusic
{
public:
	AudioMusic(std::string filename);
	~AudioMusic();
	void play(bool priority = true);
	void setPaused(bool pause);
	void togglePaused();
private:
	bool paused;
	const std::string filename;
	Mix_Music* audioHandle;
};

#endif