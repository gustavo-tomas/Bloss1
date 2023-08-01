#include "audio.hpp"

#include "soloud/include/soloud.h"
#include "soloud/include/soloud_wav.h"
#include "soloud/include/soloud_thread.h"

namespace bls
{
    Audio::Audio()
    {
        // Declare some variables
        SoLoud::Soloud soloud; // Engine core
        SoLoud::Wav wav;       // One sample source

        // Initialize SoLoud (automatic back-end selection)
        // also, enable visualization for FFT calc
        soloud.init();
        soloud.setVisualizationEnable(1);

        std::cout << "Welcome to Soloud!\n";

        // Load a wave file
        auto res = wav.load("bloss1/assets/sounds/test.wav");
        if (res != SoLoud::SOLOUD_ERRORS::SO_NO_ERROR)
        {
            std::cout << "OOPS\n";
            exit(1);
        }
        // wav.setLooping(1);                          // Tell SoLoud to loop the sound

        i32 handle1 = soloud.play(wav);             // Play it
        // soloud.setVolume(handle1, 0.5f);            // Set volume; 1.0f is "normal"
        // soloud.setPan(handle1, -0.2f);              // Set pan; -1 is left, 1 is right
        // soloud.setRelativePlaySpeed(handle1, 0.7f); // Play a bit slower; 1.0f is normal

        // Wait for voice to finish
        while (soloud.getVoiceCount() > 0)
            SoLoud::Thread::sleep(100); // Still going, sleep for a bit

        soloud.stop(handle1); // stop the wind sound

        // Clean up SoLoud
        soloud.deinit();
    }

    Audio::~Audio()
    {

    }
};
