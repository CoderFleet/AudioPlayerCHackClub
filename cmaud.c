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
    bool looping;
    bool muted;
    Uint8 prevVolume;
} AudioData;

void audioCallback(void* userdata, Uint8* stream, int len) {
    AudioData* audio = (AudioData*)userdata;
    if (audio->position == audio->length) {
        if (audio->looping) {
            audio->position = 0;
        } else {
            return;
        }
    }

    Uint32 remaining = audio->length - audio->position;
    Uint32 length = (len > remaining) ? remaining : len;

    if (!audio->muted) {
        SDL_MixAudioFormat(stream, audio->buffer + audio->position, AUDIO_S8, length, SDL_MIX_MAXVOLUME);
    } else {
        SDL_memset(stream, 0, len);
    }

    audio->position += length;

    if (length < len) {
        SDL_memset(stream + length, 0, len - length);
    }
}

void displayPlaybackStatus(AudioData* audio, SDL_AudioDeviceID deviceId) {
    int volume;
    SDL_GetAudioDeviceVolume(deviceId, &volume);

    printf("\033[2J\033[H"); // Found This On Internet XD
    printf("Now playing: %s\n", argv[1]);
    printf("Press 'q' to quit\n");
    printf("Use arrow keys (< and >) to adjust volume\n");
    printf("Press 'p' to pause/resume playback\n");
    printf("Press 's' to stop playback\n");
    printf("Press 'm' to toggle mute\n");
    printf("Press 'l' to toggle loop playback\n");

    if (audio->muted) {
        printf("Muted\n");
    } else {
        printf("Volume: %d%%\n", volume * 100 / SDL_MIX_MAXVOLUME);
    }

    printf("Playback Position: %.2f seconds / Total Duration: %.2f seconds\n",
        (float)audio->position / 1000, (float)audio->length / 1000);
    printf("Looping: %s\n", audio->looping ? "Enabled" : "Disabled");
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

    AudioData audio;
    audio.buffer = NULL;
    audio.length = 0;
    audio.position = 0;
    audio.looping = false;
    audio.muted = false;
    audio.prevVolume = SDL_MIX_MAXVOLUME;

    SDL_AudioDeviceID deviceId = 0;

    if (SDL_LoadWAV(argv[1], &wavSpec, &wavBuffer, &wavLength) == NULL) {
        printf("Failed to load WAV file '%s'! SDL_Error: %s\n", argv[1], SDL_GetError());
        SDL_Quit();
        return 1;
    }

    audio.buffer = wavBuffer;
    audio.length = wavLength;

    wavSpec.callback = audioCallback;
    wavSpec.userdata = &audio;

    deviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (deviceId == 0) {
        printf("Failed to open audio device! SDL_Error: %s\n", SDL_GetError());
        SDL_FreeWAV(audio.buffer);
        SDL_Quit();
        return 1;
    }

    SDL_PauseAudioDevice(deviceId, 0);

    bool quit = false;
    bool paused = false;
    int volume = SDL_MIX_MAXVOLUME;

    SDL_Event event;
    while (!quit) {
        displayPlaybackStatus(&audio, deviceId);

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
                        SDL_CloseAudioDevice(deviceId);
                        SDL_FreeWAV(audio.buffer);
                        audio.buffer = NULL;
                        audio.length = 0;
                        audio.position = 0;
                        printf("Playback stopped\n");
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
                    case SDLK_m:
                        audio.muted = !audio.muted;
                        if (audio.muted) {
                            SDL_GetAudioDeviceVolume(deviceId, &audio.prevVolume);
                            SDL_SetAudioDeviceVolume(deviceId, 0);
                            printf("Muted\n");
                        } else {
                            SDL_SetAudioDeviceVolume(deviceId, audio.prevVolume);
                            printf("Unmuted\n");
                        }
                        break;
                    case SDLK_l:
                        audio.looping = !audio.looping;
                        if (audio.looping) {
                            printf("Looping enabled\n");
                        } else {
                            printf("Looping disabled\n");
                        }
                        break;
                }
                break;
        }

        SDL_Delay(100);
    }

    SDL_CloseAudioDevice(deviceId);
    SDL_FreeWAV(audio.buffer);
    SDL_Quit();
    return 0;
}
