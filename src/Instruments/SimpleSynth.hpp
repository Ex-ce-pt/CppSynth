#ifndef CPPSYNTH_INSTRUMENTS_SIMPLESYNTH_H
#define CPPSYNTH_INSTRUMENTS_SIMPLESYNTH_H

#include "Instrument.hpp"

namespace CppSynth::Instruments {
    
    class SimpleSynth : public Instrument {
    public:
        enum class WaveType {
            SINE,
            SAW,
            TRIANGLE
        };

    public:
        SimpleSynth(WaveType wave = WaveType::SINE);

        void renderForEvent(std::span<Sample> buffer, const NoteRenderParams& params) noexcept override;

    private:
        GeneratorFn m_Generator = nullptr;
    };

}

#endif /* CPPSYNTH_INSTRUMENTS_SIMPLESYNTH_H */