#include "Tests.hpp"

#include "song.hpp"

#include <algorithm>

// otherwise it's unreadable
using namespace CppSynth;
using namespace Instruments;
using namespace Test;

std::vector<TimedEvent> EVENTS_MELODY;
std::vector<TimedEvent> EVENTS_BASS;
std::shared_ptr<TrackList> tracks;
SimpleSynth leadSynth(SimpleSynth::WaveType::SINE);
SimpleSynth bassSynth(SimpleSynth::WaveType::TRIANGLE);

SimpleSynth testSynth;

std::shared_ptr<TrackList> Test::prepareTestSimple(uint32_t sampleRate) {
    EVENTS_MELODY.clear();
    EVENTS_BASS.clear();

    for (const auto& note : MELODY) {
        EVENTS_MELODY.push_back({ { EventType::KEY_ON, note.pitch }, static_cast<uint64_t>(note.startSec * sampleRate) });
        EVENTS_MELODY.push_back({ { EventType::KEY_OFF, note.pitch }, static_cast<uint64_t>((note.startSec + note.durationSec) * sampleRate) });
    }

    // to make sure events are in order
    std::sort(
        EVENTS_MELODY.begin(),
        EVENTS_MELODY.end(),
        [](const TimedEvent& lhs, const TimedEvent& rhs) {
            return lhs.timeSamples < rhs.timeSamples;
        }
    );

    tracks = std::make_shared<TrackList>();
    tracks->emplace(&leadSynth, EVENTS_MELODY);

    return tracks;
}

std::shared_ptr<TrackList> Test::prepareTestComplex(uint32_t sampleRate) {
    EVENTS_MELODY.clear();
    EVENTS_BASS.clear();

    for (const auto& note : MELODY) {
        EVENTS_MELODY.push_back({ { EventType::KEY_ON, note.pitch }, static_cast<uint64_t>(note.startSec * sampleRate) });
        EVENTS_MELODY.push_back({ { EventType::KEY_OFF, note.pitch }, static_cast<uint64_t>((note.startSec + note.durationSec) * sampleRate) });
    }

    // to make sure events are in order
    std::sort(
        EVENTS_MELODY.begin(),
        EVENTS_MELODY.end(),
        [](const TimedEvent& lhs, const TimedEvent& rhs) {
            return lhs.timeSamples < rhs.timeSamples;
        }
    );

    for (const auto& note : BASS) {
        EVENTS_BASS.push_back({ { EventType::KEY_ON, note.pitch }, static_cast<uint64_t>(note.startSec * sampleRate) });
        EVENTS_BASS.push_back({ { EventType::KEY_OFF, note.pitch }, static_cast<uint64_t>((note.startSec + note.durationSec) * sampleRate) });
    }

    std::sort(
        EVENTS_BASS.begin(),
        EVENTS_BASS.end(),
        [](const TimedEvent& lhs, const TimedEvent& rhs) {
            return lhs.timeSamples < rhs.timeSamples;
        }
    );

    tracks = std::make_shared<TrackList>();
    tracks->emplace(&leadSynth, EVENTS_MELODY);
    tracks->emplace(&bassSynth, EVENTS_BASS);
    tracks->emplace(&testSynth, std::vector<TimedEvent>{ { EventType::KEY_OFF, 0.0, 5 * sampleRate } });

    return tracks;
}
