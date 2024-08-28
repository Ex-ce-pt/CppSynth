#include "AudioSystem.hpp"
#include "Sample.hpp"
#include "../Instruments/SimpleSynth.hpp"

#include <chrono>
#include <algorithm>
#include <execution>
#include <iostream>

CppSynth::AudioSystem::AudioSystem(const AudioSystemSettings& settings)
	: m_Settings(settings), m_AudioBuffers(settings.numberOfAudioBuffers)
{ }

CppSynth::AudioSystem::~AudioSystem()
{
	if (!m_Initialized) return;

	close();
}

CppSynth::AudioSystem::AudioSystemStatus CppSynth::AudioSystem::init() noexcept
{
	// Open default device
	m_CurrentDevice = alcOpenDevice(NULL);
	if (!m_CurrentDevice) {
		std::cerr << "Unable to open default device" << std::endl;
		return AudioSystemStatus::FAILED_TO_CONNECT_DEVICE;
	}

	// Create context
	m_CurrentContext = alcCreateContext(m_CurrentDevice, NULL);
	if (!alcMakeContextCurrent(m_CurrentContext)) {
		std::cerr << "Failed to make context current" << std::endl;
		return AudioSystemStatus::FAILED_TO_CREATE_CONTEXT;
	}

	alGenBuffers(static_cast<ALsizei>(m_Settings.numberOfAudioBuffers), m_AudioBuffers.Data());
	alGenSources(1, &m_Source);

	m_Initialized = true;

	return AudioSystemStatus::SUCCESS;
}

void CppSynth::AudioSystem::close() noexcept
{
	if (!m_Initialized) return;

	alDeleteSources(1, &m_Source);
	alDeleteBuffers(static_cast<ALsizei>(m_Settings.numberOfAudioBuffers), m_AudioBuffers.Data());
	alcMakeContextCurrent(NULL);
	alcDestroyContext(m_CurrentContext);
	alcCloseDevice(m_CurrentDevice);

	m_Initialized = false;
	m_Playing = false;
}

void CppSynth::AudioSystem::setTimeSamples(uint64_t timeSamples) noexcept
{
	m_TimeSamples = timeSamples;
}

void CppSynth::AudioSystem::setTimeSeconds(double timeSeconds) noexcept
{
	m_TimeSamples = timeSeconds * m_Settings.sampleRate;
}

void CppSynth::AudioSystem::setTimeChangeCallback(TimeChangeCallback cb) noexcept
{
	m_TimeChangeCallback = cb;
}

void CppSynth::AudioSystem::setTracks(const std::unordered_map<Instruments::Instrument*, std::vector<TimedEvent>>& tracks) noexcept
{
	m_Tracks = tracks;
}

void CppSynth::AudioSystem::play() noexcept
{

	for (size_t i = 0; i < m_Settings.numberOfAudioBuffers; i++) {
		fillAudioBuffer(m_AudioBuffers[i], m_TimeSamples);
		m_TimeSamples += m_Settings.audioBufferSize;
	}

	alSourceQueueBuffers(m_Source, static_cast<ALsizei>(m_Settings.numberOfAudioBuffers), m_AudioBuffers.Data());

	// Arbitrary sleep time to let the audio buffers initialize fully, to be deleted
	std::this_thread::sleep_for(std::chrono::seconds(1));

	// Start streaming
	alSourcePlay(m_Source);
	m_Playing = true;

	m_AudioPlayerThread = std::make_unique<std::thread>([this]() {
		double durationSamples = getTracksTotalDurationSamples();

		// Wait for the sound to finish playing
		ALint state;
		do {
			alGetSourcei(m_Source, AL_SOURCE_STATE, &state);

			// If the last buffer has already been filled, there is no reason to refill anything
			if (m_TimeSamples >= durationSamples) continue;

			ALint numBuffersProcessed;
			alGetSourcei(m_Source, AL_BUFFERS_PROCESSED, &numBuffersProcessed);

			for (size_t i = 0; i < numBuffersProcessed; i++) {
				ALuint buffer;
				alSourceUnqueueBuffers(m_Source, 1, &buffer);

				fillAudioBuffer(buffer, m_TimeSamples);
				m_TimeSamples += m_Settings.audioBufferSize;

				alSourceQueueBuffers(m_Source, 1, &buffer);

				if (m_TimeChangeCallback) m_TimeChangeCallback(m_TimeSamples);
			}

			// NOTE: Arbitrary sleep time to lower the CPU usage, to be deleted
			std::this_thread::sleep_for(std::chrono::milliseconds(20));

		} while (state == AL_PLAYING && m_Playing);
		m_Playing = false;
	});
}

void CppSynth::AudioSystem::fillAudioBuffer(ALuint bufferID, size_t bufferStartTimeSamples) {
	auto t = std::chrono::steady_clock::now();

	CppSynth::ABuffer buffer(m_Settings.audioBufferSize);
	memset(buffer.Data(), SAMPLE_EQUILIBRIUM, m_Settings.audioBufferSize * sizeof(Sample));

	for (const auto& [instrument, events] : m_Tracks) {
		for (const auto& tevent : events) {
			if (bufferStartTimeSamples <= tevent.timeSamples && tevent.timeSamples < bufferStartTimeSamples + m_Settings.audioBufferSize) {
				instrument->onEvent(tevent);
			}
		}

		instrument->fillAudioBuffer(buffer, bufferStartTimeSamples, m_Settings);
	}

	alBufferData(bufferID, AUDIO_FORMAT, buffer.Data(), static_cast<ALsizei>(m_Settings.audioBufferSize * sizeof(Sample)), static_cast<ALsizei>(m_Settings.sampleRate));

	std::cout << "Buffer Fill Time: " << ((std::chrono::steady_clock::now() - t).count() * 0.001 * 0.001) << "ms\n";
}

uint64_t CppSynth::AudioSystem::getTracksTotalDurationSamples()
{
	uint64_t maxEventTimeSamples = 0;

	for (const auto& [_, events] : m_Tracks) {
		maxEventTimeSamples = std::max_element(std::execution::par, events.begin(), events.end(), [](const TimedEvent& lhs, const TimedEvent& rhs) { return lhs.timeSamples < rhs.timeSamples; })->timeSamples;
	}

	return maxEventTimeSamples + m_Settings.paddingAfterSongEndSeconds * m_Settings.sampleRate;
}

void CppSynth::AudioSystem::pause() noexcept
{
	m_Playing = false;
}

void CppSynth::AudioSystem::joinPlayerThread() noexcept
{
	if (!m_AudioPlayerThread) return;

	m_AudioPlayerThread->join();
}

inline bool CppSynth::AudioSystem::isPlaying() noexcept
{
	return m_Playing;
}

inline uint64_t CppSynth::AudioSystem::getTimeSamples() noexcept
{
	return m_TimeSamples;
}

double CppSynth::AudioSystem::getTimeSeconds() noexcept
{
	return m_TimeSamples / static_cast<double>(m_Settings.sampleRate);
}
