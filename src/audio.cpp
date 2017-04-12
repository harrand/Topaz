#include "audio.hpp"

AudioClip::AudioClip(const std::string& filename): filename(filename)
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

AudioSource::AudioSource(const std::string& filename, const Vector3F& position): AudioClip(filename), position(position){}

void AudioSource::update(Player& relativeTo)
{
	Vector3F perp = relativeTo.getCamera().getForward().cross(this->getPosition() - relativeTo.getPosition());
	float dir = perp.normalised().dot(relativeTo.getCamera().getUp());
	float right = 1.0f, left = 1.0f;
	if(!std::isnan(dir))
	{
		right  = - dir;
		left = dir;
	}
	float distance = (this->getPosition() - relativeTo.getPosition()).length() / 1000;
	Mix_Volume(this->getChannel(), 128 / ((distance * distance) + 1));
	Mix_SetPanning(this->getChannel(), left * 255, right * 255);
}

const Vector3F& AudioSource::getPosition() const
{
	return this->position;
}

void AudioSource::setPosition(Vector3F position)
{
	this->position = position;
}

AudioMusic::AudioMusic(const std::string& filename): filename(filename), paused(false)
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