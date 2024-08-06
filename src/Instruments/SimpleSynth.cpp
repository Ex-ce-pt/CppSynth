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

void CppSynth::Instruments::SimpleSynth::fillAudioBuffer(ABuffer& buffer, size_t bufferStartTimeSamples, const std::vector<Note>& notes, const AudioSystem::AudioSystemSettings& audioSettings) noexcept
{
    if (m_Generator == nullptr) return;

    for (const Note& note : notes) {
        // Converting data in seconds into samples
        size_t noteStartSample = static_cast<size_t>(note.startSec * audioSettings.SampleRate);
        size_t noteDurationSample = static_cast<size_t>(note.durationSec * audioSettings.SampleRate);

        // Should the note play? (should any part of the note be in the buffer?)
        if (!(noteStartSample + noteDurationSample > bufferStartTimeSamples && noteStartSample < bufferStartTimeSamples + buffer.Size())) {
            continue;
        }

        // Calculating how much samples would the note actually contribute to rendering the audio buffer
        size_t leftBoundWithinBufferSamples = std::max(noteStartSample, bufferStartTimeSamples);
        size_t rightBoundWithinBufferSamples = std::min(noteStartSample + noteDurationSample, bufferStartTimeSamples + buffer.Size());
        size_t numberOfSamplesToPlay = rightBoundWithinBufferSamples - leftBoundWithinBufferSamples;

        // Render the data into a separate buffer
        ABuffer noteDataBuffer(numberOfSamplesToPlay);

        // TODO: make abstract
        m_Generator(noteDataBuffer, 0.3, A4 * pow(2, note.pitch / 12.0), audioSettings.SampleRate, leftBoundWithinBufferSamples);

        // Merge the resulting buffer with the rest of the audio
        size_t noteDataBufferOffsetInAudioBufferSamples = leftBoundWithinBufferSamples - bufferStartTimeSamples;
        for (size_t i = 0; i < numberOfSamplesToPlay; i++) {
            buffer[noteDataBufferOffsetInAudioBufferSamples + i] += noteDataBuffer[i];
        }
    }
}