#ifndef CPPSYNTH_INSTRUMENTS_INSTRUMENT_HPP
#define CPPSYNTH_INSTRUMENTS_INSTRUMENT_HPP

#include "../AudioSystem/Sample.hpp"
#include "../AudioSystem/AudioSystem.hpp"
#include "../Utils/Array.hpp"
#include <vector>

namespace CppSynth::Instruments {

    struct Note {
        double startSec;
        double durationSec;
        double pitch;
    };

    class Instrument {
    public:
        // ANY generator function must be able to fill any arbitrary buffer with any section of its correspondent audio
        using GeneratorFn = void(*)(ABuffer& buffer, double amp, double freq, uint64_t sampleRate, uint64_t offsetSample);

    public:
        void onEvent(const TimedEvent& tevent);
        
        virtual void fillAudioBuffer(ABuffer& buffer, uint64_t bufferStartTimeSamples, const AudioSystem::AudioSystemSettings& audioSettings) noexcept = 0;
        
    protected:
        std::vector<TimedEvent> m_CurrentEvents;
    };

}

#endif /* CPPSYNTH_INSTRUMENTS_INSTRUMENT_HPP */