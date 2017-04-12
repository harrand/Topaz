#ifndef AUDIO_HPP
#define AUDIO_HPP
#include <string>
#include "SDL_mixer.h"
#include "player.hpp"

class AudioClip
{
public:
	AudioClip(const std::string& filename);
	~AudioClip();
	void play();
	virtual void update(Player& relativeTo){}
	int getChannel() const;
private:
	int channel;
	const std::string filename;
	Mix_Chunk* audioHandle;
};

class AudioSource: public AudioClip
{
public:
	AudioSource(const std::string& filename, const Vector3F& position);
	void update(Player& relativeTo);
	const Vector3F& getPosition() const;
	void setPosition(Vector3F position);
private:
	Vector3F position;
};

class AudioMusic
{
public:
	AudioMusic(const std::string& filename);
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