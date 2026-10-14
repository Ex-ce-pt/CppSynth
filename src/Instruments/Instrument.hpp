#ifndef CPPSYNTH_INSTRUMENTS_INSTRUMENT_HPP
#define CPPSYNTH_INSTRUMENTS_INSTRUMENT_HPP

#include "../AudioSystem/Sample.hpp"
#include "../AudioSystem/AudioSystem.hpp"
#include <vector>
#include <span>

namespace CppSynth::Instruments {

    class Instrument {
    public:
        struct NoteRenderParams {
            const TimedEvent& timedEvent = {};
            size_t generatedSignalStartOffset = 0; // Think "phase" for a wave
            double amplitude = 0.0; // TODO: should it be called "gain"?
            double pitch = 0.0;
            uint64_t sampleRate = 0;
            bool stereo = false;
        };

        // ANY generator function must be able to fill any arbitrary buffer with any section of its correspondent audio.
        // Put simply, given a start time, the generator function has to play its audio into the given buffer.
        // `generatedSignalStartOffset` is the index of the starting sample of the resulting audio, not the first index of the `buffer` that has to be filled.
        using GeneratorFn = void(*)(std::span<Sample> buffer, const NoteRenderParams& params);

    public:
        virtual ~Instrument() = default;

        void onEvent(const TimedEvent& tevent);
        
        void fillAudioBuffer(std::span<Sample> buffer, uint64_t bufferStartTimeSamples, const AudioSystem::AudioSystemSettings& audioSettings) noexcept;

        virtual void renderForEvent(std::span<Sample> buffer, const NoteRenderParams& params) noexcept = 0;

    protected:
        std::vector<TimedEvent> m_CurrentEvents;
    };

}

#endif /* CPPSYNTH_INSTRUMENTS_INSTRUMENT_HPP */