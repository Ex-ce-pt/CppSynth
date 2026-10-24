#include "SimpleSynth.hpp"
#include "../Utils/Utils.hpp"

using CppSynth::Sample;
using NoteRenderParams = CppSynth::Instruments::Instrument::NoteRenderParams;

// Generators

static void gen_sin(std::span<Sample> buffer, const NoteRenderParams& params) noexcept
{
    size_t phaseSample = params.generatedSignalStartOffset;
    for (size_t i = 0; i < buffer.size(); i++) {
        double timeSec = static_cast<double>(phaseSample + i) / params.sampleRate;
        Sample value = static_cast<Sample>(params.amplitude * sin(2.0 * PI * params.pitch * timeSec));
        buffer[i] += value;
    }
}

static void gen_saw(std::span<Sample> buffer, const NoteRenderParams& params) noexcept
{
    size_t phaseSample = params.generatedSignalStartOffset;
    for (size_t i = 0; i < buffer.size(); i++) {
        double timeSec = static_cast<double>(phaseSample + i) / params.sampleRate;
        double x = params.pitch * timeSec;
        Sample value = static_cast<Sample>((x - floor(x)) * params.amplitude);
        buffer[i] += value;
    }
}

static void gen_triangle(std::span<Sample> buffer, const NoteRenderParams& params) noexcept
{
    size_t phaseSample = params.generatedSignalStartOffset;
    for (size_t i = 0; i < buffer.size(); i++) {
        double timeSec = static_cast<double>(phaseSample + i) / params.sampleRate;
        double x = params.pitch * timeSec;
        Sample value = static_cast<Sample>((4 * abs(x + 0.25 - floor(x + 0.75)) - 1) * params.amplitude);
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

void CppSynth::Instruments::SimpleSynth::renderForEvent(std::span<Sample> buffer, const NoteRenderParams& params) noexcept
{
    if (m_Generator == nullptr) {
        return;
    }

    m_Generator(buffer, params);
}