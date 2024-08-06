#ifndef CPPSYNTH_INSTRUMENTS_SIMPLESYNTH_H
#define CPPSYNTH_INSTRUMENTS_SIMPLESYNTH_H

#include "Instrument.hpp"

namespace CppSynth::Instruments {
    
    class SimpleSynth : public Instrument {
    public:
        enum class WaveType {
            SINE,
            SAW
        };

    public:
        SimpleSynth(WaveType wave = WaveType::SINE);

        virtual void fillAudioBuffer(ABuffer& buffer, size_t bufferStartTimeSamples, const AudioSystem::AudioSystemSettings& audioSettings) noexcept override;

    private:
        GeneratorFn m_Generator = nullptr;
    };

}

#endif /* CPPSYNTH_INSTRUMENTS_SIMPLESYNTH_H */