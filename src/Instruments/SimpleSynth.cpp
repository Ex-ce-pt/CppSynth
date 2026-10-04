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

        auto firstSampleToFill = buffer.begin();
        auto lastSampleToFill = buffer.end();

        if (tevent.event.type == EventType::KEY_OFF) { // TODO: also render audio appropriately if the event ends in the middle of the buffer
            // Positive - OFF event inside the buffer
            // Negative - OFF event before the buffer started, skip
            int64_t timeBetweenOffEventAndBufferStartSamples = static_cast<int64_t>(tevent.timeSamples - bufferStartTimeSamples);
            
            if (timeBetweenOffEventAndBufferStartSamples < 0) {
                continue;
            }

            lastSampleToFill = std::next(buffer.begin(), timeBetweenOffEventAndBufferStartSamples);

            // Problem: cannot calculate the phase now when the event has been overwritten
            continue;
        }

        // If positive (event occurred before the buffer started), indicates a phase of the signal about to be generated.
        // If negative (event occurred after the buffer started), indicates that the event starts somewhere in the middle of the buffer.
        int64_t timeBetweenEventAndBufferStartSamples = static_cast<int64_t>(bufferStartTimeSamples - tevent.timeSamples);
        
        uint64_t startingPhase = timeBetweenEventAndBufferStartSamples;
        

        if (timeBetweenEventAndBufferStartSamples < 0) {
            startingPhase = 0;
            firstSampleToFill = std::next(firstSampleToFill, -timeBetweenEventAndBufferStartSamples);
        }

        // Prepare section of buffer this note is supposed to render to
        std::span<Sample> noteDataBufferSpan(firstSampleToFill, lastSampleToFill); // TODO: should not be end iterator ideally

        // TODO: make abstract
        m_Generator(noteDataBufferSpan, 0.3, A4 * pow(2, tevent.event.pitch / 12.0), audioSettings.sampleRate, startingPhase);
    }
}