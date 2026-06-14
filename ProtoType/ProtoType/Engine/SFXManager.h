#pragma once
#include <filesystem>
#include <unordered_map>
#include <vector>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

namespace CS230
{
    struct AudioData
    {
        Uint8* buffer;
        Uint32 length;
    };

    struct PlayingSound
    {
        AudioData* audio;
        Uint32 position;
        bool loop;
        int volume;
        bool isBGM;
    };

    class SFXManager
    {
    public:
        ~SFXManager();

        bool Load(const std::filesystem::path& file_name);

        void PlaySFX(const std::filesystem::path& file_name, float speed = 1.0f, bool loop = false, int volume = SDL_MIX_MAXVOLUME);
        void PlayBGM(const std::filesystem::path& file_name, float speed = 1.0f, int volume = SDL_MIX_MAXVOLUME);

        void StopSFX();
        void StopBGM();
        void StopAll();

        void SetSFXVolume(int volume);
        void SetBGMVolume(int volume);
        void SetMasterVolume(int volume);

        int GetSFXVolume() const { return sfxVolume; }
        int GetBGMVolume() const { return bgmVolume; }
        int GetMasterVolume() const { return masterVolume; }

        void Unload();

    private:
        std::unordered_map<std::filesystem::path, AudioData> sounds;
        std::vector<PlayingSound> playingSounds;
        bool audioInitialized = false;
        SDL_AudioDeviceID deviceId = 0;
        SDL_AudioSpec targetSpec;

        int masterVolume = SDL_MIX_MAXVOLUME;
        int sfxVolume = SDL_MIX_MAXVOLUME;
        int bgmVolume = SDL_MIX_MAXVOLUME;

        void InitAudio();
        static void AudioCallback(void* userdata, Uint8* stream, int len);
    };
}
