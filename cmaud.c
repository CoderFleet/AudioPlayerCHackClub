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

    SDL_AudioSpec wavSpec;
    Uint32 wavLength;
    Uint8 *wavBuffer;

    AudioData audio;
    audio.buffer = NULL;
    audio.length = 0;
    audio.position = 0;

    SDL_AudioDeviceID deviceId = 0;

    printf("Usage:\n");
    printf("  %s <audio_file> - Play the specified audio file\n", argv[0]);
    printf("Controls:\n");
    printf("  'q' - Quit\n");
    printf("  'p' - Pause/Resume playback\n");
    printf("  's' - Select a new audio file\n");
    printf("  '<' - Decrease volume\n");
    printf("  '>' - Increase volume\n");
    printf("  Left Arrow  - Seek backward 5 seconds\n");
    printf("  Right Arrow - Seek forward 5 seconds\n");

    SDL_Event event;
    bool quit = false;
    bool paused = false;
    int volume = SDL_MIX_MAXVOLUME / 2;

    while (!quit) {
        SDL_PollEvent(&event);
        switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        printf("Exiting...\n");
                        quit = true;
                        break;
                    case SDLK_p:
                        paused = !paused;
                        if (paused) {
                            SDL_PauseAudioDevice(deviceId, 1);
                            printf("Playback paused\n");
                        } else {
                            SDL_PauseAudioDevice(deviceId, 0);
                            printf("Playback resumed\n");
                        }
                        break;
                    case SDLK_s:
                        if (audio.buffer != NULL) {
                            SDL_FreeWAV(audio.buffer);
                            audio.buffer = NULL;
                            audio.length = 0;
                            audio.position = 0;
                        }
                        if (argc < 2) {
                            printf("Usage: %s <audio_file>\n", argv[0]);
                            break;
                        }
                        if (SDL_LoadWAV(argv[1], &wavSpec, &wavBuffer, &wavLength) == NULL) {
                            printf("Failed to load WAV! SDL_Error: %s\n", SDL_GetError());
                            break;
                        }
                        audio.buffer = wavBuffer;
                        audio.length = wavLength;
                        audio.position = 0;
                        paused = false;

                        if (deviceId != 0) {
                            SDL_CloseAudioDevice(deviceId);
                        }

                        wavSpec.callback = audioCallback;
                        wavSpec.userdata = &audio;
                        deviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, SDL_AUDIO_ALLOW_ANY_CHANGE);
                        if (deviceId == 0) {
                            printf("Failed to open audio device! SDL_Error: %s\n", SDL_GetError());
                            SDL_FreeWAV(audio.buffer);
                            audio.buffer = NULL;
                            audio.length = 0;
                            audio.position = 0;
                            break;
                        }
                        SDL_PauseAudioDevice(deviceId, 0);

                        printf("Now playing: %s\n", argv[1]);
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
                    case SDLK_LEFT:
                        if (audio.position >= 5000) {
                            audio.position -= 5000;
                            printf("Seeking backward 5 seconds\n");
                        } else {
                            audio.position = 0;
                            printf("Seeking to start of audio\n");
                        }
                        break;
                    case SDLK_RIGHT:
                        if (audio.position + 5000 <= audio.length) {
                            audio.position += 5000;
                            printf("Seeking forward 5 seconds\n");
                        } else {
                            audio.position = audio.length;
                            printf("Seeking to end of audio\n");
                        }
                        break;
                }
                break;
        }

        if (audio.length > 0) {
            printf("Playback Position: %.2f seconds / Total Duration: %.2f seconds\n",
                (float)audio.position / 1000, (float)audio.length / 1000);
        }

        SDL_Delay(100);
    }

    SDL_CloseAudioDevice(deviceId);
    SDL_FreeWAV(audio.buffer);
    SDL_Quit();
    return 0;
}
