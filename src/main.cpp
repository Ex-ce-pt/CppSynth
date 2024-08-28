#include "AudioSystem/AudioSystem.hpp"

#include <cstdio>

#include "song.hpp"
#include "Instruments/SimpleSynth.hpp"

std::vector<CppSynth::TimedEvent> EVENTS_MELODY;
std::vector<CppSynth::TimedEvent> EVENTS_BASS;
std::unordered_map<CppSynth::Instruments::Instrument*, std::vector<CppSynth::TimedEvent>> tracks;
CppSynth::Instruments::SimpleSynth synth(CppSynth::Instruments::SimpleSynth::WaveType::SINE);
CppSynth::Instruments::SimpleSynth bassSynth(CppSynth::Instruments::SimpleSynth::WaveType::TRIANGLE);

void prepareTest(uint32_t sampleRate) {
    EVENTS_MELODY.clear();
    EVENTS_BASS.clear();

    for (const auto& note : MELODY) {
        EVENTS_MELODY.push_back({ { CppSynth::EventType::KEY_ON, note.pitch }, static_cast<uint64_t>(note.startSec * sampleRate) });
        EVENTS_MELODY.push_back({ { CppSynth::EventType::KEY_OFF, note.pitch }, static_cast<uint64_t>((note.startSec + note.durationSec) * sampleRate) });
    }

    for (const auto& note : BASS) {
        EVENTS_BASS.push_back({ { CppSynth::EventType::KEY_ON, note.pitch }, static_cast<uint64_t>(note.startSec * sampleRate) });
        EVENTS_BASS.push_back({ { CppSynth::EventType::KEY_OFF, note.pitch }, static_cast<uint64_t>((note.startSec + note.durationSec) * sampleRate) });
    }

    tracks = {
        { &synth, EVENTS_MELODY },
        { &bassSynth, EVENTS_BASS }
    };
}

int main() {
    prepareTest(CppSynth::AudioSystem::DEFAULT_SAMPLE_RATE);

    printf("Initializing the Audio System [DEFAULT settings]...\n");

    // Initialize OpenAL
    CppSynth::AudioSystem audioSystem;
    auto status = audioSystem.Init();
    switch (status) {
        case CppSynth::AudioSystem::AudioSystemStatus::SUCCESS:
            printf("Audio System Initialized\n");
            break;

        case CppSynth::AudioSystem::AudioSystemStatus::FAILED_TO_CONNECT_DEVICE:
            printf("Failed to connect the default device\n");
            return -1;

        case CppSynth::AudioSystem::AudioSystemStatus::FAILED_TO_CREATE_CONTEXT:
            printf("Failed to create context for the default device\n");
            return -2;

        default:
            printf("Undefined status of the Audio System: %i\n", status);
            return -10;
    }
    
    audioSystem.SetTimeSeconds(0);
    audioSystem.SetTracks(tracks);
    audioSystem.Play();
    audioSystem.JoinPlayerThread();

    printf("Finished playing\n");
    printf("Closing the Audio System...\n");
    audioSystem.Close();

    return 0;
}
