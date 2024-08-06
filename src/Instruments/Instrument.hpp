#ifndef CPPSYNTH_INSTRUMENTS_INSTRUMENT_HPP
#define CPPSYNTH_INSTRUMENTS_INSTRUMENT_HPP

#include <vector>
#include "../AudioSystem/Sample.hpp"
#include "../AudioSystem/Buffer.hpp"
#include "../AudioSystem/AudioSystem.hpp"

namespace CppSynth::Instruments {

    struct Note {
        double startSec;
        double durationSec;
        double pitch;
    };

    class Instrument {
    public:
        // ANY generator function must be able to fill any arbitrary buffer with any section of its correspondent audio
        using GeneratorFn = void(*)(ABuffer& buffer, double amp, double freq, size_t sampleRate, size_t offsetSample);

    public:
        void message(const Message& msg);
        
        virtual void fillAudioBuffer(ABuffer& buffer, size_t bufferStartTimeSamples, const AudioSystem::AudioSystemSettings& audioSettings) noexcept = 0;
        
    protected:
        std::vector<Message> messages;
    };

}

#endif /* CPPSYNTH_INSTRUMENTS_INSTRUMENT_HPP */