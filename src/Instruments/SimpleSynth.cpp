#include "SimpleSynth.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

#include "../Utils/Utils.hpp"

// Generators

static void gen_sin(CppSynth::ABuffer& buffer, double amp, double freq, size_t sampleRate, size_t offsetSample = 0) noexcept
{
    for (size_t i = 0; i < buffer.Size(); i++) {
        double timeSec = static_cast<double>(offsetSample + i) / sampleRate;
        Sample value = static_cast<Sample>(amp * SAMPLE_MAX_VALUE * sin(2.0 * M_PI * freq * timeSec));
        buffer[i] = value;
    }
}

static void gen_saw(CppSynth::ABuffer& buffer, double amp, double freq, size_t sampleRate, size_t offsetSample = 0) noexcept
{
    for (size_t i = 0; i < buffer.Size(); i++) {
        double timeSec = static_cast<double>(offsetSample + i) / sampleRate;
        double x = freq * timeSec;
        Sample value = static_cast<Sample>((x - floor(x)) * amp * SAMPLE_MAX_VALUE);
        buffer[i] += value;
    }
}

//////////

CppSynth::Instruments::SimpleSynth::SimpleSynth(WaveType wave)
{
    switch (wave) {
    case WaveType::SINE: m_Generator = gen_sin; break;
    case WaveType::SAW: m_Generator = gen_saw; break;
    default: return;
    }
}

void CppSynth::Instruments::SimpleSynth::fillAudioBuffer(ABuffer& buffer, size_t bufferStartTimeSamples, const AudioSystem::AudioSystemSettings& audioSettings) noexcept
{
    if (m_Generator == nullptr) return;

    for (const Message& msg : messages) {
        if (msg.type == MessageType::KEY_OFF) continue;

        // Render the data into a separate buffer
        ABuffer noteDataBuffer(buffer.Size());

        // TODO: make abstract
        m_Generator(noteDataBuffer, 0.3, A4 * pow(2, 0 / 12.0), audioSettings.SampleRate, bufferStartTimeSamples);

        // Merge the resulting buffer with the rest of the audio
        for (size_t i = 0; i < buffer.Size(); i++) {
            buffer[i] += noteDataBuffer[i];
        }
    }
}