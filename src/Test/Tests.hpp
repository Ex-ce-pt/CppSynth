#ifndef CPPSYNTH_TEST_TESTS_HPP
#define CPPSYNTH_TEST_TESTS_HPP

#include "../Instruments/SimpleSynth.hpp"

#include <memory>

namespace Test {

    // move to AudioSystem?
    using TrackList = std::unordered_map<CppSynth::Instruments::Instrument*, std::vector<CppSynth::TimedEvent>>;
    
    std::shared_ptr<TrackList> prepareTestSimple(uint32_t sampleRate);
    std::shared_ptr<TrackList> prepareTestComplex(uint32_t sampleRate);

}

#endif /* CPPSYNTH_TEST_TESTS_HPP */
