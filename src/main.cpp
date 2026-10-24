#include "AudioSystem/AudioSystem.hpp"
#include "Debug/Log.hpp"

#include "Test/Tests.hpp"

int main() {
    //auto tracks = Test::prepareTestComplex(CppSynth::AudioSystem::DEFAULT_SAMPLE_RATE);
    auto tracks = Test::prepareTestSimple(CppSynth::AudioSystem::DEFAULT_SAMPLE_RATE);

    CppSynth::Debug::log("Initializing the Audio System [DEFAULT settings]...\n");

    // Initialize OpenAL
    CppSynth::AudioSystem audioSystem;
    auto status = audioSystem.init();

    switch (status) {
        case CppSynth::AudioSystem::AudioSystemStatus::SUCCESS:
            CppSynth::Debug::log("Audio System Initialized\n");
            break;

        case CppSynth::AudioSystem::AudioSystemStatus::FAILED_TO_CONNECT_DEVICE:
            CppSynth::Debug::log("Failed to connect the default device\n");
            return -1;

        case CppSynth::AudioSystem::AudioSystemStatus::FAILED_TO_CREATE_CONTEXT:
            CppSynth::Debug::log("Failed to create context for the default device\n");
            return -2;

        default:
            CppSynth::Debug::log("Undefined status of the Audio System: ");
            CppSynth::Debug::log((int64_t) status);
            CppSynth::Debug::log("\n");
            return -10;
    }
    
    audioSystem.setTimeSeconds(0);
    audioSystem.setTracks(tracks);
    audioSystem.play();
    audioSystem.joinPlayerThread();

    CppSynth::Debug::log("Finished playing\n");
    CppSynth::Debug::log("Closing the Audio System...\n");
    audioSystem.close();

    return 0;
}
