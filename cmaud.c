#include <SDL2/SDL.h>
#include <stdio.h>

void printWelcomeMessage() {
    printf("░█████╗░███╗░░░███╗░█████╗░██╗░░░██╗██████╗░\n");
    printf("██╔══██╗████╗░████║██╔══██╗██║░░░██║██╔══██╗\n");
    printf("██║░░╚═╝██╔████╔██║███████║██║░░░██║██║░░██║\n");
    printf("██║░░██╗██║╚██╔╝██║██╔══██║██║░░░██║██║░░██║\n");
    printf("╚█████╔╝██║░╚═╝░██║██║░░██║╚██████╔╝██████╔╝\n");
    printf("░╚════╝░╚═╝░░░░░╚═╝╚═╝░░╚═╝░╚═════╝░╚═════╝░\n");
    printf("Welcome to cmaud - Command Line Audio Player\n\n");
}

int main(int argc, char* argv[]) {
    printWelcomeMessage();

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    if (argc < 2) {
        printf("Usage: %s <audio_file>\n", argv[0]);
        SDL_Quit();
        return 1;
    }

    SDL_AudioSpec wavSpec;
    Uint32 wavLength;
    Uint8 *wavBuffer;

    if (SDL_LoadWAV(argv[1], &wavSpec, &wavBuffer, &wavLength) == NULL) {
        printf("Failed to load WAV! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    printf("Loaded WAV file: %s\n", argv[1]);
    printf("Audio format: %d\n", wavSpec.format);
    printf("Channels: %d\n", wavSpec.channels);
    printf("Frequency: %d\n", wavSpec.freq);

    SDL_FreeWAV(wavBuffer);
    SDL_Quit();
    return 0;
}
