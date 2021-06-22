#include "ggwave/ggwave.h"

#include "ggwave-common.h"
#include "ggwave-common-sdl2.h"

#include <SDL.h>

#include <cstdio>
#include <string>

#include <mutex>
#include <thread>
#include <iostream>

int main(int argc, char** argv) {
    printf("Usage: %s [-cN] [-pN] [-tN] [-lN]\n", argv[0]);
    printf("    -cN - select capture device N\n");
    printf("    -pN - select playback device N\n");
    printf("    -tN - transmission protocol\n");
    printf("    -lN - fixed payload length of size N, N in [1, %d]\n", GGWave::kMaxLengthFixed);
    printf("\n");

    auto argm = parseCmdArguments(argc, argv);
    int captureId = argm["c"].empty() ? 0 : std::stoi(argm["c"]);
    int playbackId = argm["p"].empty() ? 0 : std::stoi(argm["p"]);
    int txProtocol = argm["t"].empty() ? 1 : std::stoi(argm["t"]);
    int payloadLength = argm["l"].empty() ? -1 : std::stoi(argm["l"]);

    if (GGWave_init(playbackId, captureId, payloadLength) == false) {
        fprintf(stderr, "Failed to initialize GGWave\n");
        return -1;
    }

    auto ggWave = GGWave_instance();

    printf("Available Tx protocols:\n");
    const auto & protocols = GGWave::getTxProtocols();
    for (const auto & protocol : protocols) {
        printf("    -t%d : %s\n", protocol.first, protocol.second.name);
    }

    if (txProtocol < 0 || txProtocol > (int) ggWave->getTxProtocols().size()) {
        fprintf(stderr, "Unknown Tx protocol %d\n", txProtocol);
        return -3;
    }

    printf("Selecting Tx protocol %d\n", txProtocol);

    std::mutex mutex;
    std::thread inputThread([&]() {
        std::string inputOld = "";
        while (true) {
            std::string input;
            std::cout << "Enter text: ";
            getline(std::cin, input);
            if (input.empty()) {
                std::cout << "Re-sending ... " << std::endl;
                input = inputOld;
            } else {
                std::cout << "Sending ... " << std::endl;
            }
            {
                std::lock_guard<std::mutex> lock(mutex);
                ggWave->init(input.size(), input.data(), ggWave->getTxProtocol(txProtocol), 10);
            }
            inputOld = input;
        }
    });

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        {
            std::lock_guard<std::mutex> lock(mutex);
            GGWave_mainLoop();
        }
    }

    inputThread.join();

    GGWave_deinit();

    SDL_CloseAudio();
    SDL_Quit();

    return 0;
}
