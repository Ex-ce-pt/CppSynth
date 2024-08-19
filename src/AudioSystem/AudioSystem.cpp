#include "AudioSystem.hpp"
#include "Sample.hpp"
#include "../Instruments/SimpleSynth.hpp"

#include <chrono>
#include <iostream>

CppSynth::AudioSystem::AudioSystem(const AudioSystemSettings& settings)
    : m_Settings(settings), m_AudioBuffers(settings.NumberOfAudioBuffers)
{ }

CppSynth::AudioSystem::~AudioSystem()
{
    if (!m_Initialized) return;

    Close();
}

CppSynth::AudioSystem::AudioSystemStatus CppSynth::AudioSystem::Init() noexcept
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

    alGenBuffers(static_cast<ALsizei>(m_Settings.NumberOfAudioBuffers), m_AudioBuffers.Data());
    alGenSources(1, &m_Source);

    m_Initialized = true;

	return AudioSystemStatus::SUCCESS;
}

void CppSynth::AudioSystem::Close() noexcept
{
    if (!m_Initialized) return;

    alDeleteSources(1, &m_Source);
    alDeleteBuffers(static_cast<ALsizei>(m_Settings.NumberOfAudioBuffers), m_AudioBuffers.Data());
    alcMakeContextCurrent(NULL);
    alcDestroyContext(m_CurrentContext);
    alcCloseDevice(m_CurrentDevice);

    m_Initialized = false;
    m_Playing = false;
}

// DEBUG

#include "song.hpp"

std::vector<CppSynth::TimedEvent> EVENTS;
CppSynth::Instruments::SimpleSynth synth(CppSynth::Instruments::SimpleSynth::WaveType::TRIANGLE);

static void transformNotesIntoEvents(uint64_t sampleRate) {
    EVENTS.clear();

    for (const auto& note : NOTES) {
        EVENTS.push_back({ { CppSynth::EventType::KEY_ON, note.pitch }, static_cast<uint64_t>(note.startSec * sampleRate) });
        EVENTS.push_back({ { CppSynth::EventType::KEY_OFF, note.pitch }, static_cast<uint64_t>((note.startSec + note.durationSec) * sampleRate) });
    }
}

////////

static void fillAudioBuffer(ALuint bufferID, size_t bufferStartTimeSamples, const CppSynth::AudioSystem::AudioSystemSettings& audioSettings) {
    CppSynth::ABuffer buffer(audioSettings.AudioBufferSize);
    memset(buffer.Data(), SAMPLE_EQUILIBRIUM, audioSettings.AudioBufferSize * sizeof(Sample));

    for (const auto& tevent : EVENTS) {
        if (bufferStartTimeSamples <= tevent.TimeSamples && tevent.TimeSamples < bufferStartTimeSamples + audioSettings.AudioBufferSize) {
            synth.onEvent(tevent);
        }
    }

    synth.fillAudioBuffer(buffer, bufferStartTimeSamples, audioSettings);

    alBufferData(bufferID, AUDIO_FORMAT, buffer.Data(), static_cast<ALsizei>(audioSettings.AudioBufferSize * sizeof(Sample)), static_cast<ALsizei>(audioSettings.SampleRate));
}

void CppSynth::AudioSystem::SetTimeSamples(uint64_t timeSamples) noexcept
{
    m_TimeSamples = timeSamples;
}

void CppSynth::AudioSystem::SetTimeSeconds(double timeSeconds) noexcept
{
    m_TimeSamples = timeSeconds * m_Settings.SampleRate;
}

void CppSynth::AudioSystem::SetTimeChangeCallback(TimeChangeCallback cb) noexcept
{
    m_TimeChangeCallback = cb;
}

void CppSynth::AudioSystem::Play() noexcept
{

    transformNotesIntoEvents(m_Settings.SampleRate);
    for (size_t i = 0; i < m_Settings.NumberOfAudioBuffers; i++) {
        fillAudioBuffer(m_AudioBuffers[i], m_TimeSamples, m_Settings);
        m_TimeSamples += m_Settings.AudioBufferSize;
    }

    alSourceQueueBuffers(m_Source, static_cast<ALsizei>(m_Settings.NumberOfAudioBuffers), m_AudioBuffers.Data());

    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Start streaming
    alSourcePlay(m_Source);
    m_Playing = true;

    double duration = NOTES.crbegin()->startSec + NOTES.crbegin()->durationSec + 1; // Duration in seconds
    long double amplitude = 0.3;

    m_AudioPlayerThread = std::make_unique<std::thread>([this, duration]() {
        // Wait for the sound to finish playing
        ALint state;
        do {
            alGetSourcei(m_Source, AL_SOURCE_STATE, &state);

            // If the last buffer has already been filled, there is no reason to refill anything
            if (m_TimeSamples >= duration * m_Settings.SampleRate) continue;

            ALint numBuffersProcessed;
            alGetSourcei(m_Source, AL_BUFFERS_PROCESSED, &numBuffersProcessed);

            for (size_t i = 0; i < numBuffersProcessed; i++) {
                ALuint buffer;
                alSourceUnqueueBuffers(m_Source, 1, &buffer);

                fillAudioBuffer(buffer, m_TimeSamples, m_Settings);
                m_TimeSamples += m_Settings.AudioBufferSize;

                alSourceQueueBuffers(m_Source, 1, &buffer);

                if (m_TimeChangeCallback) m_TimeChangeCallback(m_TimeSamples);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(20));

        } while (state == AL_PLAYING && m_Playing);
        m_Playing = false;
    });
}

void CppSynth::AudioSystem::Pause() noexcept
{
    m_Playing = false;
}

void CppSynth::AudioSystem::JoinPlayerThread() noexcept
{
    if (!m_AudioPlayerThread) return;

    m_AudioPlayerThread->join();
}

inline bool CppSynth::AudioSystem::IsPlaying() noexcept
{
    return m_Playing;
}

inline uint64_t CppSynth::AudioSystem::GetTimeSamples() noexcept
{
    return m_TimeSamples;
}

double CppSynth::AudioSystem::GetTimeSeconds() noexcept
{
    return m_TimeSamples / static_cast<double>(m_Settings.SampleRate);
}
