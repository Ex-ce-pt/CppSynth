#include "AudioSystem.hpp"

#include <chrono>
#include <thread>

#include <iostream>
#include "Sample.hpp"
#include "../Instruments/SimpleSynth.hpp"

CppSynth::AudioSystem::AudioSystem(const AudioSystemSettings& settings)
    : m_Settings(settings), m_AudioBuffers(settings.NumberOfAudioBuffers)
{ }

CppSynth::AudioSystem::~AudioSystem()
{
	if (m_Initialized)
	{
		Close();
	}
}

bool CppSynth::AudioSystem::Init() noexcept
{
    // Open default device
    m_CurrentDevice = alcOpenDevice(NULL);
    if (!m_CurrentDevice) {
        std::cerr << "Unable to open default device" << std::endl;
        return false;
    }

    // Create context
    m_CurrentContext = alcCreateContext(m_CurrentDevice, NULL);
    if (!alcMakeContextCurrent(m_CurrentContext)) {
        std::cerr << "Failed to make context current" << std::endl;
        return false;
    }

    alGenBuffers(static_cast<ALsizei>(m_Settings.NumberOfAudioBuffers), m_AudioBuffers.Data());
    alGenSources(1, &m_Source);

    m_Initialized = true;

	return true;
}

// DEBUG

std::vector<CppSynth::Instruments::Note> NOTES = {
    { 0.0, 0.5, 0.0 },
    { 1.0, 0.5, 0.0 },
    { 1.5, 0.25, -2.0 },
    { 1.75, 0.25, 0.0 },
    { 2.5, 0.5, 0 },
    { 3.0, 0.25, 3.0 },
    { 3.25, 0.5, 5.0 },
    { 4.0, 1.0, 6.0 },
    { 5.0, 0.5, 5.0 },
    { 5.5, 0.5, 3.0 },
    { 6.0, 0.5, 0.0 },
    { 6.5, 0.5, -2.0 },
    { 7.0, 0.25, 0.0 },
    { 7.25, 0.25, -2.0 },
    { 7.5, 0.25, 0.0 },
    { 7.75, 0.25, 3.0 },

    { 8.0, 0.5, 0.0 },
    { 9.0, 0.5, 0.0 },
    { 9.5, 0.25, 3.0 },
    { 9.75, 0.25, 0.0 },
    { 10.5, 0.5, 0 },
    { 11.0, 0.25, 3.0 },
    { 11.25, 0.5, 5.0 },
    { 12.0, 1.0, 6.0 },
    { 13.0, 0.5, 5.0 },
    { 13.5, 0.5, 3.0 },
    { 14.0, 0.5, 0.0 },
    { 14.5, 0.5, -2.0 },
    { 15.0, 0.25, 0.0 },
    { 15.25, 0.25, -2.0 },
    { 15.5, 0.25, 0.0 },
    { 15.75, 0.25, 3.0 },

};

std::vector<CppSynth::TimedEvent> EVENTS;
CppSynth::Instruments::SimpleSynth synth(CppSynth::Instruments::SimpleSynth::WaveType::SINE);

static void transformNotesIntoEvents(size_t sampleRate) {
    EVENTS.clear();

    //return;
    for (const auto& note : NOTES) {
        EVENTS.push_back({ { CppSynth::EventType::KEY_ON, note.pitch }, static_cast<size_t>(note.startSec * sampleRate) });
        EVENTS.push_back({ { CppSynth::EventType::KEY_OFF, note.pitch }, static_cast<size_t>((note.startSec + note.durationSec) * sampleRate) });
    }
}

////////

static void fillAudioBuffer(ALuint bufferID, size_t bufferStartTimeSamples, const CppSynth::AudioSystem::AudioSystemSettings& audioSettings) {
    CppSynth::ABuffer buffer(audioSettings.AudioBufferSize);
    memset(buffer.Data(), SAMPLE_EQUILIBRIUM, audioSettings.AudioBufferSize * sizeof(Sample));

    for (const auto& te : EVENTS) {
        if (bufferStartTimeSamples <= te.timeSamples && te.timeSamples < bufferStartTimeSamples + audioSettings.AudioBufferSize) {
            synth.onEvent(te);
        }
    }

    synth.fillAudioBuffer(buffer, bufferStartTimeSamples, audioSettings);

    alBufferData(bufferID, AUDIO_FORMAT, buffer.Data(), static_cast<ALsizei>(audioSettings.AudioBufferSize * sizeof(Sample)), static_cast<ALsizei>(audioSettings.SampleRate));
}

void CppSynth::AudioSystem::Play(uint64_t startTime) noexcept
{
    m_TimeSamples = startTime;

    transformNotesIntoEvents(m_Settings.SampleRate);
    for (size_t i = 0; i < m_Settings.NumberOfAudioBuffers; i++) {
        fillAudioBuffer(m_AudioBuffers[i], m_TimeSamples, m_Settings);
        m_TimeSamples += m_Settings.AudioBufferSize;
    }

    alSourceQueueBuffers(m_Source, m_Settings.NumberOfAudioBuffers, m_AudioBuffers.Data());

    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Start streaming
    alSourcePlay(m_Source);
    m_Playing = true;

    double duration = NOTES.crbegin()->startSec + NOTES.crbegin()->durationSec + 1; // Duration in seconds
    long double amplitude = 0.3;

    // Wait for the sound to finish playing
    ALint state;
    do {
        alGetSourcei(m_Source, AL_SOURCE_STATE, &state);

        // If the last buffer las already been filled, there is no reason to refill anything
        if (m_TimeSamples >= duration * m_Settings.SampleRate) continue;

        ALint numBuffersProcessed;
        alGetSourcei(m_Source, AL_BUFFERS_PROCESSED, &numBuffersProcessed);

        for (size_t i = 0; i < numBuffersProcessed; i++) {
            ALuint buffer;
            alSourceUnqueueBuffers(m_Source, 1, &buffer);

            fillAudioBuffer(buffer, m_TimeSamples, m_Settings);
            m_TimeSamples += m_Settings.AudioBufferSize;

            alSourceQueueBuffers(m_Source, 1, &buffer);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(20));

    } while (state == AL_PLAYING);
}

void CppSynth::AudioSystem::Close() noexcept
{
    if (!m_Initialized) return;

    alDeleteSources(1, &m_Source);
    alDeleteBuffers(m_Settings.NumberOfAudioBuffers, m_AudioBuffers.Data());
    alcMakeContextCurrent(NULL);
    alcDestroyContext(m_CurrentContext);
    alcCloseDevice(m_CurrentDevice);

    m_Initialized = false;
}

inline bool CppSynth::AudioSystem::IsPlaying() noexcept
{
    return m_Playing;
}
