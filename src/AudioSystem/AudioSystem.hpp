#ifndef CPPSYNTH_AUDIOSYSTEM_AUDIOSYSTEM_HPP
#define CPPSYNTH_AUDIOSYSTEM_AUDIOSYSTEM_HPP

#include "Buffer.hpp"
#include <AL/al.h>
#include <AL/alc.h>
#include <atomic>
#include <memory>
#include <thread>
#include <unordered_map>

namespace CppSynth {

	enum class EventType {
		KEY_ON,
		KEY_OFF
	};

	struct Event {
		EventType Type;
		double Pitch;
	};

	struct TimedEvent {
		Event Event;
		uint64_t TimeSamples;
	};

	// FWD DECL
	namespace Instruments {
		class Instrument;
	}

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

		enum class AudioSystemStatus {
			SUCCESS,
			FAILED_TO_CONNECT_DEVICE,
			FAILED_TO_CREATE_CONTEXT
		};

	public:
		using TimeChangeCallback = void(*)(uint64_t newTime);

		AudioSystem(const AudioSystemSettings& settings = {});
		~AudioSystem();

		AudioSystemStatus Init() noexcept;
		void Close() noexcept;

		void SetTimeSamples(uint64_t timeSamples) noexcept;
		void SetTimeSeconds(double timeSeconds) noexcept;
		void SetTimeChangeCallback(TimeChangeCallback cb) noexcept;
		void SetTracks(const std::unordered_map<Instruments::Instrument*, std::vector<TimedEvent>>& tracks) noexcept; // NOTE: don't copy?
		void Play() noexcept;
		void Pause() noexcept;
		void JoinPlayerThread() noexcept;

		inline bool IsPlaying() noexcept;
		inline uint64_t GetTimeSamples() noexcept;
		double GetTimeSeconds() noexcept;

	private:
		void FillAudioBuffer(ALuint bufferID, size_t bufferStartTimeSamples);

	private:
		AudioSystemSettings m_Settings;
		ALCdevice* m_CurrentDevice = nullptr;
		ALCcontext* m_CurrentContext = nullptr;
		ALuint m_Source = 0;
		Buffer<ALuint> m_AudioBuffers;

		bool m_Initialized = false;
		std::atomic_bool m_Playing = false;
		uint64_t m_TimeSamples = 0;
		std::unique_ptr<std::thread> m_AudioPlayerThread;
		TimeChangeCallback m_TimeChangeCallback = nullptr;
		std::unordered_map<Instruments::Instrument*, std::vector<TimedEvent>> m_Tracks;
	};

}

#endif /* CPPSYNTH_AUDIOSYSTEM_AUDIOSYSTEM_HPP */