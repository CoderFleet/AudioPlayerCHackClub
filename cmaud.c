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

typedef struct {
    Uint8* buffer;
    Uint32 length;
    Uint32 position;
} AudioData;

void audioCallback(void* userdata, Uint8* stream, int len) {
    AudioData* audio = (AudioData*)userdata;
    if (audio->position == audio->length) {
        return;
    }

    Uint32 remaining = audio->length - audio->position;
    Uint32 length = (len > remaining) ? remaining : len;
    SDL_memcpy(stream, audio->buffer + audio->position, length);
    audio->position += length;

    if (length < len) {
        SDL_memset(stream + length, 0, len - length);
    }
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

    AudioData audio;
    audio.buffer = wavBuffer;
    audio.length = wavLength;
    audio.position = 0;

    wavSpec.callback = audioCallback;
    wavSpec.userdata = &audio;

    if (SDL_OpenAudio(&wavSpec, NULL) < 0) {
        printf("SDL_OpenAudio failed! SDL_Error: %s\n", SDL_GetError());
        SDL_FreeWAV(wavBuffer);
        SDL_Quit();
        return 1;
    }

    SDL_PauseAudio(0);
    while (audio.position < audio.length) {
        SDL_Delay(100);
    }

    SDL_FreeWAV(wavBuffer);
    SDL_CloseAudio();
    SDL_Quit();
    return 0;
}
