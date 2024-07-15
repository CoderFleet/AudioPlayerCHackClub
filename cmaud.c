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
    SDL_MixAudioFormat(stream, audio->buffer + audio->position, AUDIO_S8, length, SDL_MIX_MAXVOLUME);
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

    SDL_AudioDeviceID deviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (deviceId == 0) {
        printf("Failed to open audio device! SDL_Error: %s\n", SDL_GetError());
        SDL_FreeWAV(wavBuffer);
        SDL_Quit();
        return 1;
    }

    SDL_PauseAudioDevice(deviceId, 0);

    printf("\nPress 'q' to quit\n");
    printf("Use arrow keys (< and >) to adjust volume\n");

    int volume = SDL_MIX_MAXVOLUME;
    SDL_Event event;
    while (audio.position < audio.length) {
        SDL_PollEvent(&event);
        switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        printf("Exiting...\n");
                        audio.position = audio.length;
                        break;
                    case SDLK_COMMA:
                        if (volume > 0) {
                            volume -= SDL_MIX_MAXVOLUME / 10;
                            SDL_SetAudioDeviceVolume(deviceId, volume);
                            printf("Volume: %d%%\n", volume * 100 / SDL_MIX_MAXVOLUME);
                        }
                        break;
                    case SDLK_PERIOD:
                        if (volume < SDL_MIX_MAXVOLUME) {
                            volume += SDL_MIX_MAXVOLUME / 10;
                            SDL_SetAudioDeviceVolume(deviceId, volume);
                            printf("Volume: %d%%\n", volume * 100 / SDL_MIX_MAXVOLUME);
                        }
                        break;
                }
                break;
        }
        SDL_Delay(100);
    }

    SDL_CloseAudioDevice(deviceId);
    SDL_FreeWAV(wavBuffer);
    SDL_Quit();
    return 0;
}
