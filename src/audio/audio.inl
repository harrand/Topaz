#include <chrono>
#include <thread>

namespace tz::audio
{
	template<typename Audio>
	void play_async(Audio&& clip)
	{
		using namespace std::chrono_literals;
		// Play clip for the length of the audio clip plus another 10 milliseconds.
		auto play_clip = [](auto clip){clip.play(); std::this_thread::sleep_for(operator""ms(static_cast<unsigned long long>(clip.get_audio_length()) + 10ull));};
		std::thread(play_clip, clip).detach();
	}
}