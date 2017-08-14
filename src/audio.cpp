#include "audio.hpp"

AudioClip::AudioClip(std::string filename): filename(std::move(filename))
{
	this->audio_handle = Mix_LoadWAV(this->filename.c_str());
}

AudioClip::AudioClip(const AudioClip& copy): AudioClip(copy.getFileName()){}

AudioClip::AudioClip(AudioClip&& move): filename(move.getFileName()), audio_handle(move.audio_handle)
{
	move.audio_handle = nullptr;
}

AudioClip::~AudioClip()
{
	// Cannot guarantee that Mix_FreeChunk(nullptr) doesn't lead to UB (this happens if this instance was moved to another) so put a check in here to prevent crashing
	if(this->audio_handle != nullptr)
		Mix_FreeChunk(this->audio_handle);
}

void AudioClip::play()
{
	this->channel = Mix_PlayChannel(-1, this->audio_handle, 0);
}

int AudioClip::getChannel() const
{
	return this->channel;
}

const std::string& AudioClip::getFileName() const
{
	return this->filename;
}

const Mix_Chunk* AudioClip::getAudioHandle() const
{
	return this->audio_handle;
}

AudioSource::AudioSource(std::string filename, Vector3F position): AudioClip(filename), position(std::move(position)){}

void AudioSource::update(Player& relativeTo)
{
	const Vector3F sourcePosition = this->position;
	const Vector3F listenerPosition = relativeTo.getPosition();
	const Vector3F forward = relativeTo.getCamera().getForward();
	const Vector3F up = relativeTo.getCamera().getUp();
	float proportionRight = (forward.cross(up).normalised().dot((sourcePosition - listenerPosition).normalised()) + 1) / 2;
	if(sourcePosition == listenerPosition)
		proportionRight = 0.5;
	float right = proportionRight;
	float left = 1 - proportionRight;
	Mix_SetPanning(this->getChannel(), left * 255, right * 255);
	float distance = (this->getPosition() - relativeTo.getPosition()).length() / 100;
	Mix_Volume(this->getChannel(), 128 / ((distance * distance) + 1));
}

Vector3F& AudioSource::getPositionR()
{
	return this->position;
}

const Vector3F& AudioSource::getPosition() const
{
	return this->position;
}

void AudioSource::setPosition(Vector3F position)
{
	this->position = position;
}

AudioMusic::AudioMusic(std::string filename): filename(std::move(filename)), paused(false)
{
	this->audio_handle = Mix_LoadMUS(this->filename.c_str());
}
AudioMusic::AudioMusic(const AudioMusic& copy): AudioMusic(copy.getFileName()){}

AudioMusic::AudioMusic(AudioMusic&& move): filename(move.getFileName()), audio_handle(move.audio_handle)
{
	move.audio_handle = nullptr;
}

AudioMusic::~AudioMusic()
{
	Mix_FreeMusic(this->audio_handle);
}

const std::string& AudioMusic::getFileName() const
{
	return this->filename;
}

Mix_Music*& AudioMusic::getAudioHandle()
{
	return this->audio_handle;
}

void AudioMusic::play(bool priority) const
{
	if(priority || Mix_PlayingMusic() != 0)
		Mix_PlayMusic(this->audio_handle, -1);
}

void AudioMusic::setPaused(bool pause)
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