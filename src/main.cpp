#include "AudioSystem/AudioSystem.hpp"

#include <cstdio>

int main() {
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
    audioSystem.Play();
    audioSystem.JoinPlayerThread();

    printf("Finished playing\n");
    printf("Closing the Audio System...");
    audioSystem.Close();

    return 0;
}
