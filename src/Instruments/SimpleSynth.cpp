#include "SimpleSynth.hpp"
#include "../Utils/Utils.hpp"

using CppSynth::Sample;

// Generators

static void gen_sin(std::span<Sample> buffer, double amp, double freq, uint64_t sampleRate, size_t phaseSample = 0) noexcept
{
    for (size_t i = 0; i < buffer.size(); i++) {
        double timeSec = static_cast<double>(phaseSample + i) / sampleRate;
        Sample value = static_cast<Sample>(amp * sin(2.0 * PI * freq * timeSec));
        buffer[i] += value;
    }
}

static void gen_saw(std::span<Sample> buffer, double amp, double freq, uint64_t sampleRate, size_t phaseSample = 0) noexcept
{
    for (size_t i = 0; i < buffer.size(); i++) {
        double timeSec = static_cast<double>(phaseSample + i) / sampleRate;
        double x = freq * timeSec;
        Sample value = static_cast<Sample>((x - floor(x)) * amp);
        buffer[i] += value;
    }
}

static void gen_triangle(std::span<Sample> buffer, double amp, double freq, uint64_t sampleRate, size_t phaseSample = 0) noexcept
{
    for (size_t i = 0; i < buffer.size(); i++) {
        double timeSec = static_cast<double>(phaseSample + i) / sampleRate;
        double x = freq * timeSec;
        Sample value = static_cast<Sample>((4 * abs(x + 0.25 - floor(x + 0.75)) - 1) * amp);
        buffer[i] += value;
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

void CppSynth::Instruments::SimpleSynth::fillAudioBuffer(std::span<Sample> buffer, uint64_t bufferStartTimeSamples, const AudioSystem::AudioSystemSettings& audioSettings) noexcept
{
    if (m_Generator == nullptr) {
        return;
    }

    for (const TimedEvent& tevent : m_CurrentEvents) {
        if (tevent.event.type == EventType::KEY_OFF) { // TODO: also render audio appropriately if the event ends in the middle of the buffer
            continue;
        }

        // If positive, indicates a phase of the signal about to be generated.
        // If negative, indicates that the event starts somewhere in the middle of the buffer.
        int64_t timeBetweenEventAndBufferStartSamples = static_cast<int64_t>(bufferStartTimeSamples - tevent.timeSamples);
        
        uint64_t startingPhase = timeBetweenEventAndBufferStartSamples;
        size_t startingBufferIdx = 0;

        if (timeBetweenEventAndBufferStartSamples < 0) {
            startingPhase = 0;
            startingBufferIdx = -timeBetweenEventAndBufferStartSamples;
        }

        // Prepare section of buffer this note is supposed to render to
        std::span<Sample> noteDataBufferSpan(buffer.begin() + startingBufferIdx, buffer.end()); // TODO: should not be end iterator ideally

        // TODO: make abstract
        m_Generator(noteDataBufferSpan, 0.3, A4 * pow(2, tevent.event.pitch / 12.0), audioSettings.sampleRate, startingPhase);
    }
}