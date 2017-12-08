#include "audio.hpp"
#include <thread>
#include <chrono>

AudioClip::AudioClip(std::string filename): filename(std::move(filename)), audio_handle(Mix_LoadWAV(this->filename.c_str())){}

AudioClip::AudioClip(const AudioClip& copy): AudioClip(copy.get_file_name()){}

AudioClip::AudioClip(AudioClip&& move): filename(move.get_file_name()), audio_handle(move.audio_handle)
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
	constexpr int first_free_unreserved_channel = -1;
	constexpr int number_of_loops = 0;
	this->channel = Mix_PlayChannel(first_free_unreserved_channel, this->audio_handle, number_of_loops);
}

int AudioClip::get_channel() const
{
	return this->channel;
}

Uint32 AudioClip::get_audio_length() const
{
	Uint32 points = 0, frames = 0;
	int frequency = 0;
	Uint16 format = 0;
	int channels;
	if(!Mix_QuerySpec(&frequency, &format, &channels))
	{
		tz::util::log::error("Attempt to query AudioClip yielded invalid query. Is tz::audio initialised?");
		return 0;
	}
	// bytes / sample_size == sample_points
	points = (this->audio_handle->alen / ((format & 0xFF) / 8));
	// sample_points / channels = sample_frames
	frames = points / channels;
	// sample_frames * 1000 / frequency = audio length in milliseconds
	return frames * 1000 / frequency;
}

const std::string& AudioClip::get_file_name() const
{
	return this->filename;
}

AudioSource::AudioSource(std::string filename): AudioClip(filename){}

void AudioSource::update(const Vector3F& source_position, const Camera& relative_to)
{
	const Vector3F listener_position = relative_to.position;
	const Vector3F forward = relative_to.forward();
	const Vector3F displacement = source_position - listener_position;
	// a.b = |a||b|*cos(A)
	// so A = acos(a dot b / |a||b|)
	float angle = std::acos(forward.dot(displacement) / (forward.length() * displacement.length()));
	Mix_SetPosition(this->get_channel(), static_cast<Sint16>(angle), static_cast<Uint8>(255 * displacement.length() / relative_to.far_clip));
}

AudioMusic::AudioMusic(std::string filename): filename(std::move(filename)), paused(false)
{
	this->audio_handle = Mix_LoadMUS(this->filename.c_str());
}

AudioMusic::AudioMusic(const AudioMusic& copy): AudioMusic(copy.get_file_name()){}

AudioMusic::AudioMusic(AudioMusic&& move): filename(move.get_file_name()), audio_handle(move.audio_handle)
{
	move.audio_handle = nullptr;
}

AudioMusic::~AudioMusic()
{
	Mix_FreeMusic(this->audio_handle);
}

const std::string& AudioMusic::get_file_name() const
{
	return this->filename;
}

bool AudioMusic::is_paused() const
{
	return this->paused;
}

void AudioMusic::play(bool priority) const
{
	if(priority || Mix_PlayingMusic() != 0)
		Mix_PlayMusic(this->audio_handle, -1);
}

void AudioMusic::set_paused(bool pause)
{
	this->paused = pause;
	if(this->paused)
		Mix_PauseMusic();
	else
		Mix_ResumeMusic();
}

namespace tz::audio
{
	void initialise()
	{
		constexpr int channels = 2; // number of sound chanels in output. 2 for stereo, 1 for mono.
		constexpr int chunk_size = 4096; // bytes used per output sample
		constexpr Uint16 format = MIX_DEFAULT_FORMAT; // output sample 	format. MIX_DEFAULT_FORMAT is the same as AUDIO_S16SYS (signed 16-bit samples, in system byte order)
		// initialise sdl_mixer
		if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, format, channels, chunk_size) == -1)
			tz::util::log::error("SDL_Mixer initialisation returned an error: ", Mix_GetError(), "\n\tInitialisation of tz::audio failed.");
		else
			tz::util::log::message("Initialised tz::audio via SDL_Mixer.");
	}

	void terminate()
	{
		Mix_CloseAudio();
		tz::util::log::message("Terminated tz::audio via SDL_Mixer.");
	}
	
	void play_clip_async(const AudioClip& clip)
	{
		using namespace std::chrono_literals;
		// Play clip for the length of the audio clip plus another 10 milliseconds.
		auto play_clip = [](AudioClip clip){clip.play(); std::this_thread::sleep_for(operator""ms(static_cast<unsigned long long>(clip.get_audio_length()) + 10ull));};
		std::thread(play_clip, clip).detach();
	}
}