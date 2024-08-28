#ifndef CPPSYNTH_AUDIOSYSTEM_AUDIOSYSTEM_HPP
#define CPPSYNTH_AUDIOSYSTEM_AUDIOSYSTEM_HPP

#include "../Utils/Array.hpp"
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
		EventType type;
		double pitch;
	};

	struct TimedEvent {
		Event event;
		uint64_t timeSamples;
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
		static inline constexpr double DEFAULT_PADDING_AFTER_SONG_END_SECONDS = 1;

		struct AudioSystemSettings {
			uint64_t sampleRate = DEFAULT_SAMPLE_RATE;
			bool stereo = false;
			size_t numberOfAudioBuffers = DEFAULT_NUMBER_OF_AUDIO_BUFFERS;
			size_t audioBufferSize = DEFAULT_AUDIO_BUFFER_SIZE;
			double paddingAfterSongEndSeconds = DEFAULT_PADDING_AFTER_SONG_END_SECONDS;
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

		AudioSystemStatus init() noexcept;
		void close() noexcept;

		void setTimeSamples(uint64_t timeSamples) noexcept;
		void setTimeSeconds(double timeSeconds) noexcept;
		void setTimeChangeCallback(TimeChangeCallback cb) noexcept;
		void setTracks(const std::unordered_map<Instruments::Instrument*, std::vector<TimedEvent>>& tracks) noexcept; // NOTE: don't copy?
		void play() noexcept;
		void pause() noexcept;
		void joinPlayerThread() noexcept;

		bool isPlaying() noexcept;
		uint64_t getTimeSamples() noexcept;
		double getTimeSeconds() noexcept;
		double getIdealLatencySeconds() noexcept;

	private:
		void fillAudioBuffer(ALuint bufferID, size_t bufferStartTimeSamples);
		uint64_t getTracksTotalDurationSamples();

	private:
		AudioSystemSettings m_Settings;
		ALCdevice* m_CurrentDevice = nullptr;
		ALCcontext* m_CurrentContext = nullptr;
		ALuint m_Source = 0;
		Array<ALuint> m_AudioBuffers;

		bool m_Initialized = false;
		std::atomic_bool m_Playing = false;
		std::atomic_uint64_t m_TimeSamples = 0;
		std::unique_ptr<std::thread> m_AudioPlayerThread;
		TimeChangeCallback m_TimeChangeCallback = nullptr;
		std::unordered_map<Instruments::Instrument*, std::vector<TimedEvent>> m_Tracks;
	};

}

#endif /* CPPSYNTH_AUDIOSYSTEM_AUDIOSYSTEM_HPP */