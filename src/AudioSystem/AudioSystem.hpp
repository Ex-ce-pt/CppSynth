#ifndef CPPSYNTH_AUDIOSYSTEM_AUDIOSYSTEM_HPP
#define CPPSYNTH_AUDIOSYSTEM_AUDIOSYSTEM_HPP

#include <AL/al.h>
#include <AL/alc.h>
#include <atomic>
#include "Buffer.hpp"

namespace CppSynth {

	enum class EventType {
		KEY_ON,
		KEY_OFF
	};

	struct Event {
		EventType type;
		double pitch;
	};

	struct TimedEvent {
		Event e;
		size_t timeSamples;
	};

	class AudioSystem final
	{
	public:
		static inline constexpr size_t DEFAULT_NUMBER_OF_AUDIO_BUFFERS = 4;
		static inline constexpr size_t DEFAULT_AUDIO_BUFFER_SIZE = 2048;
		static inline constexpr uint64_t DEFAULT_SAMPLE_RATE = 44100;

		struct AudioSystemSettings {
			uint64_t SampleRate = DEFAULT_SAMPLE_RATE;
			bool Stereo = false;
			size_t NumberOfAudioBuffers = DEFAULT_NUMBER_OF_AUDIO_BUFFERS;
			size_t AudioBufferSize = DEFAULT_AUDIO_BUFFER_SIZE;
		};

	public:
		AudioSystem(const AudioSystemSettings& settings = {});
		~AudioSystem();

		bool Init() noexcept;
		void Play(uint64_t startTime = 0) noexcept;
		void Close() noexcept;

		inline bool IsPlaying() noexcept;

	private:
		AudioSystemSettings m_Settings;
		ALCdevice* m_CurrentDevice = nullptr;
		ALCcontext* m_CurrentContext = nullptr;
		ALuint m_Source = 0;
		Buffer<ALuint> m_AudioBuffers;

		bool m_Initialized = false;
		std::atomic_bool m_Playing = false;
		uint64_t m_TimeSamples = 0;
	};

}

#endif /* CPPSYNTH_AUDIOSYSTEM_AUDIOSYSTEM_HPP */