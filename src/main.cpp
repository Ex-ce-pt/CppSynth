#include <iostream>

#include "AudioSystem/AudioSystem.hpp"

int main() {
    printf("Initializing the Audio System [DEFAULT settings]...\n");

    // Initialize OpenAL
    CppSynth::AudioSystem audioSystem;
    if (!audioSystem.Init()) {
        return -1;
    }

    printf("Audio System Initialized\n");
    
    audioSystem.Play();

    audioSystem.Close();

    return 0;
}
