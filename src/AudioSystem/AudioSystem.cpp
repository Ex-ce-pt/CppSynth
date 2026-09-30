#include "AudioSystem.hpp"
#include "Sample.hpp"
#include "../Instruments/SimpleSynth.hpp"

#include <span>
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

	alGenBuffers(static_cast<ALsizei>(m_Settings.numberOfAudioBuffers), m_AudioBuffers.data());
	alGenSources(1, &m_AudioSourceId);

	m_Initialized = true;

	return AudioSystemStatus::SUCCESS;
}

void CppSynth::AudioSystem::close() noexcept
{
	if (!m_Initialized) return;

	alDeleteSources(1, &m_AudioSourceId);
	alDeleteBuffers(static_cast<ALsizei>(m_Settings.numberOfAudioBuffers), m_AudioBuffers.data());
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

void CppSynth::AudioSystem::setTracks(std::weak_ptr<std::unordered_map<Instruments::Instrument*, std::vector<TimedEvent>>> tracks) noexcept
{
	m_Tracks = tracks;
}

void CppSynth::AudioSystem::play() noexcept
{

	for (size_t i = 0; i < m_Settings.numberOfAudioBuffers; i++) {
		fillAudioBuffer(m_AudioBuffers[i], m_TimeSamples);
		m_TimeSamples += m_Settings.audioBufferSize;
	}

	alSourceQueueBuffers(m_AudioSourceId, static_cast<ALsizei>(m_Settings.numberOfAudioBuffers), m_AudioBuffers.data());

	// Arbitrary sleep time to let the audio buffers initialize fully, to be deleted
	std::this_thread::sleep_for(std::chrono::seconds(1));

	// Start streaming
	alSourcePlay(m_AudioSourceId);
	m_Playing = true;

	m_AudioPlayerThread = std::make_unique<std::thread>([this]() {
		double durationSamples = getTracksTotalDurationSamples();

		// Wait for the sound to finish playing
		ALint state;
		do {
			alGetSourcei(m_AudioSourceId, AL_SOURCE_STATE, &state);

			// If the last buffer has already been filled, there is no reason to refill anything
			if (m_TimeSamples >= durationSamples) continue;

			ALint numBuffersProcessed;
			alGetSourcei(m_AudioSourceId, AL_BUFFERS_PROCESSED, &numBuffersProcessed);

			for (size_t i = 0; i < numBuffersProcessed; i++) {
				ALuint buffer;
				alSourceUnqueueBuffers(m_AudioSourceId, 1, &buffer);

				fillAudioBuffer(buffer, m_TimeSamples);
				m_TimeSamples += m_Settings.audioBufferSize;

				alSourceQueueBuffers(m_AudioSourceId, 1, &buffer);

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

	std::vector<Sample> totalBuffer(m_Settings.audioBufferSize, SAMPLE_EQUILIBRIUM);
	
	if (!m_Tracks.expired()) {

		auto& tracks = *m_Tracks.lock();
		std::vector<Sample> instrumentBufferArena(m_Settings.audioBufferSize * tracks.size(), SAMPLE_EQUILIBRIUM);
		size_t instIdx = 0;

		for (const auto& [instrument, events] : tracks) {
			for (const auto& tevent : events) {
				if (bufferStartTimeSamples <= tevent.timeSamples && tevent.timeSamples < bufferStartTimeSamples + m_Settings.audioBufferSize) {
					instrument->onEvent(tevent);
				}
			}

			//std::vector<Sample> instBuffer(m_Settings.audioBufferSize, SAMPLE_EQUILIBRIUM);
			//std::span<Sample> instrumentBuffer(instBuffer.data(), m_Settings.audioBufferSize);
			auto instrumentBufferStartIt = instrumentBufferArena.begin() + (m_Settings.audioBufferSize * instIdx);
			auto instrumentBufferEndIt = instrumentBufferStartIt + m_Settings.audioBufferSize;
			std::span<Sample> instrumentBuffer(instrumentBufferStartIt, instrumentBufferEndIt);

			instrument->fillAudioBuffer(instrumentBuffer, bufferStartTimeSamples, m_Settings);

			instIdx++;
		}

		// TODO: try to optimize further
		for (size_t i = 0; i < instrumentBufferArena.size(); i++) {
			totalBuffer[i % m_Settings.audioBufferSize] += instrumentBufferArena[i];
		}
	}

	std::vector<NativeSampleT> nativeSampleBuffer(m_Settings.audioBufferSize);
	for (size_t i = 0; i < totalBuffer.size(); i++) {
		nativeSampleBuffer[i] = static_cast<NativeSampleT>(totalBuffer[i] * NATIVE_SAMPLE_AMPLITUDE) + NATIVE_SAMPLE_EQUILIBRIUM;
	}

	alBufferData(bufferID, AUDIO_FORMAT, nativeSampleBuffer.data(), static_cast<ALsizei>(m_Settings.audioBufferSize * sizeof(NativeSampleT)), static_cast<ALsizei>(m_Settings.sampleRate));

	std::cout << "Buffer Fill Time: " << ((std::chrono::steady_clock::now() - t).count() * 0.001 * 0.001) << "ms\n";
}

// BUG: account for the release of the final note
uint64_t CppSynth::AudioSystem::getTracksTotalDurationSamples()
{
	uint64_t maxEventTimeSamples = 0;

	if (!m_Tracks.expired()) {
		for (const auto& [_, events] : *m_Tracks.lock()) {
			uint64_t maxEventTimeSamplesForTrack = std::max_element(std::execution::par, events.begin(), events.end(), [](const TimedEvent& lhs, const TimedEvent& rhs) { return lhs.timeSamples < rhs.timeSamples; })->timeSamples;
			maxEventTimeSamples = std::max(maxEventTimeSamples, maxEventTimeSamplesForTrack);
		}
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

bool CppSynth::AudioSystem::isPlaying() noexcept
{
	return m_Playing;
}

uint64_t CppSynth::AudioSystem::getTimeSamples() noexcept
{
	return m_TimeSamples;
}

double CppSynth::AudioSystem::getTimeSeconds() noexcept
{
	return m_TimeSamples / static_cast<double>(m_Settings.sampleRate);
}

double CppSynth::AudioSystem::getIdealLatencySeconds() noexcept
{
	return static_cast<double>(m_Settings.numberOfAudioBuffers) * m_Settings.audioBufferSize / m_Settings.sampleRate;
}
