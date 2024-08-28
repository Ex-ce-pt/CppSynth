#include "SimpleSynth.hpp"
#include "../Utils/Utils.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

// Generators

static void gen_sin(CppSynth::ABuffer& buffer, double amp, double freq, uint64_t sampleRate, size_t phaseSample = 0) noexcept
{
    for (size_t i = 0; i < buffer.Size(); i++) {
        double timeSec = static_cast<double>(phaseSample + i) / sampleRate;
        Sample value = static_cast<Sample>(amp * SAMPLE_MAX_VALUE * sin(2.0 * M_PI * freq * timeSec));
        buffer[i] = value;
    }
}

static void gen_saw(CppSynth::ABuffer& buffer, double amp, double freq, uint64_t sampleRate, size_t phaseSample = 0) noexcept
{
    for (size_t i = 0; i < buffer.Size(); i++) {
        double timeSec = static_cast<double>(phaseSample + i) / sampleRate;
        double x = freq * timeSec;
        Sample value = static_cast<Sample>((x - floor(x)) * amp * SAMPLE_MAX_VALUE);
        buffer[i] = value;
    }
}

static void gen_triangle(CppSynth::ABuffer& buffer, double amp, double freq, uint64_t sampleRate, size_t phaseSample = 0) noexcept
{
    for (size_t i = 0; i < buffer.Size(); i++) {
        double timeSec = static_cast<double>(phaseSample + i) / sampleRate;
        double x = freq * timeSec;
        Sample value = static_cast<Sample>((4 * abs(x + 0.25 - floor(x + 0.75)) - 1) * amp * SAMPLE_MAX_VALUE);
        buffer[i] = value;
    }
}

//////////

CppSynth::Instruments::SimpleSynth::SimpleSynth(WaveType wave)
{
    switch (wave) {
    case WaveType::SINE: m_Generator = gen_sin; break;
    case WaveType::SAW: m_Generator = gen_saw; break;
    case WaveType::TRIANGLE: m_Generator = gen_triangle; break;
    default: return;
    }
}

void CppSynth::Instruments::SimpleSynth::fillAudioBuffer(ABuffer& buffer, uint64_t bufferStartTimeSamples, const AudioSystem::AudioSystemSettings& audioSettings) noexcept
{
    if (m_Generator == nullptr) return;

    for (const TimedEvent& tevent : m_CurrentEvents) {
        if (tevent.event.type == EventType::KEY_OFF) continue;

        // If positive, indicates a phase of the signal about to be generated.
        // If negative, indicates that the event starts somewhere in the middle of the buffer.
        int64_t timeBetweenEventAndBufferStartSamples = static_cast<int64_t>(bufferStartTimeSamples - tevent.timeSamples);
        uint64_t phaseSample = timeBetweenEventAndBufferStartSamples;
        size_t bufferOffset = 0;

        if (timeBetweenEventAndBufferStartSamples < 0) {
            phaseSample = 0;
            bufferOffset = -timeBetweenEventAndBufferStartSamples;
        }

        // Render the data into a separate buffer
        ABuffer noteDataBuffer(buffer.Size() - bufferOffset);

        // TODO: make abstract
        m_Generator(noteDataBuffer, 0.3, A4 * pow(2, tevent.event.pitch / 12.0), audioSettings.sampleRate, phaseSample);

        // Merge the resulting buffer with the rest of the audio
        for (size_t i = 0; i < noteDataBuffer.Size(); i++) {
            buffer[bufferOffset + i] += noteDataBuffer[i];
        }
    }
}