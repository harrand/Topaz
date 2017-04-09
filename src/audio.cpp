#include "audio.hpp"

AudioClip::AudioClip(std::string filename): filename(filename)
{
	this->audioHandle = Mix_LoadWAV(this->filename.c_str());
}

AudioClip::~AudioClip()
{
	Mix_FreeChunk(this->audioHandle);
}

void AudioClip::play()
{
	this->channel = Mix_PlayChannel(-1, this->audioHandle, 0);
}

int AudioClip::getChannel() const
{
	return this->channel;
}

AudioMusic::AudioMusic(std::string filename): filename(filename), paused(false)
{
	this->audioHandle = Mix_LoadMUS(this->filename.c_str());
}

AudioMusic::~AudioMusic()
{
	Mix_FreeMusic(this->audioHandle);
}

void AudioMusic::play(bool priority)
{
	// if its a priority or we're not playing music, play this.
	if(priority || Mix_PlayingMusic() != 0)
		Mix_PlayMusic(this->audioHandle, -1);
}

void AudioMusic::setPaused(bool pause = true)
{
	this->paused = pause;
	if(this->paused)
		Mix_PauseMusic();
	else
		Mix_ResumeMusic();
}

void AudioMusic::togglePaused()
{
	this->setPaused(!this->paused);
}