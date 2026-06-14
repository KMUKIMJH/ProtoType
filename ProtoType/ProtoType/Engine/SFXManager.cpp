#include <SDL2/SDL.h>
#include "SFXManager.h"
#include "Engine.h"
#include "Logger.h"

CS230::SFXManager::~SFXManager()
{
    Unload();
    if (audioInitialized)
    {
        SDL_CloseAudioDevice(deviceId);
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        audioInitialized = false;
    }
}

void CS230::SFXManager::InitAudio()
{
    if (audioInitialized) return;

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
    {
        Engine::GetLogger().LogError("Failed to init SDL Audio: " + std::string(SDL_GetError()));
        return;
    }

    SDL_zero(targetSpec);
    targetSpec.freq = 44100;
    targetSpec.format = AUDIO_S16SYS;
    targetSpec.channels = 2;
    targetSpec.samples = 4096;
    targetSpec.callback = AudioCallback;
    targetSpec.userdata = this;

    deviceId = SDL_OpenAudioDevice(nullptr, 0, &targetSpec, &targetSpec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
    if (deviceId == 0)
    {
        Engine::GetLogger().LogError("Failed to open audio device: " + std::string(SDL_GetError()));
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return;
    }

    SDL_PauseAudioDevice(deviceId, 0); // Start playback
    audioInitialized = true;
}

bool CS230::SFXManager::Load(const std::filesystem::path& file_name)
{
    if (!audioInitialized) InitAudio();
    if (!audioInitialized) return false;

    std::filesystem::path key = std::filesystem::weakly_canonical(file_name);
    if (sounds.find(key) != sounds.end())
    {
        return true;
    }

    SDL_AudioSpec wavSpec;
    Uint32 wavLength;
    Uint8* wavBuffer;

    if (SDL_LoadWAV(key.string().c_str(), &wavSpec, &wavBuffer, &wavLength) == nullptr)
    {
        Engine::GetLogger().LogError("Failed to load WAV file: " + key.string() + " - " + SDL_GetError());
        return false;
    }

    SDL_AudioCVT cvt;
    if (SDL_BuildAudioCVT(&cvt, wavSpec.format, wavSpec.channels, wavSpec.freq,
                          targetSpec.format, targetSpec.channels, targetSpec.freq) < 0)
    {
        Engine::GetLogger().LogError("Failed to build audio CVT for: " + key.string());
        SDL_FreeWAV(wavBuffer);
        return false;
    }

    AudioData data;
    if (cvt.needed)
    {
        cvt.len = wavLength;
        cvt.buf = static_cast<Uint8*>(SDL_malloc(cvt.len * cvt.len_mult));
        if (!cvt.buf)
        {
            SDL_FreeWAV(wavBuffer);
            return false;
        }
        SDL_memcpy(cvt.buf, wavBuffer, wavLength);
        SDL_ConvertAudio(&cvt);
        data.buffer = cvt.buf;
        data.length = cvt.len_cvt;
        SDL_FreeWAV(wavBuffer);
    }
    else
    {
        data.buffer = static_cast<Uint8*>(SDL_malloc(wavLength));
        SDL_memcpy(data.buffer, wavBuffer, wavLength);
        data.length = wavLength;
        SDL_FreeWAV(wavBuffer);
    }

    sounds[key] = data;
    return true;
}

void CS230::SFXManager::PlaySFX(const std::filesystem::path& file_name, [[maybe_unused]] float speed, bool loop, int volume)
{
    if (!audioInitialized) InitAudio();
    if (!audioInitialized) return;

    std::filesystem::path key = std::filesystem::weakly_canonical(file_name);
    auto it = sounds.find(key);
    if (it == sounds.end())
    {
        if (!Load(file_name))
        {
            Engine::GetLogger().LogError("SFX not loaded: " + key.string());
            return;
        }
        it = sounds.find(key);
    }

    PlayingSound snd;
    snd.audio = &it->second;
    snd.position = 0;
    snd.loop = loop;
    snd.volume = volume;
    snd.isBGM = false;

    SDL_LockAudioDevice(deviceId);
    playingSounds.push_back(snd);
    SDL_UnlockAudioDevice(deviceId);
}

void CS230::SFXManager::PlayBGM(const std::filesystem::path& file_name, [[maybe_unused]] float speed, int volume)
{
    if (!audioInitialized) InitAudio();
    if (!audioInitialized) return;

    std::filesystem::path key = std::filesystem::weakly_canonical(file_name);
    auto it = sounds.find(key);
    if (it == sounds.end())
    {
        if (!Load(file_name))
        {
            Engine::GetLogger().LogError("BGM not loaded: " + key.string());
            return;
        }
        it = sounds.find(key);
    }

    PlayingSound snd;
    snd.audio = &it->second;
    snd.position = 0;
    snd.loop = true; // BGM generally loops
    snd.volume = volume;
    snd.isBGM = true;

    SDL_LockAudioDevice(deviceId);
    playingSounds.push_back(snd);
    SDL_UnlockAudioDevice(deviceId);
}

void CS230::SFXManager::StopSFX()
{
    if (!audioInitialized) return;

    SDL_LockAudioDevice(deviceId);
    for (auto it = playingSounds.begin(); it != playingSounds.end(); )
    {
        if (!it->isBGM)
        {
            it = playingSounds.erase(it);
        }
        else
        {
            ++it;
        }
    }
    SDL_UnlockAudioDevice(deviceId);
}

void CS230::SFXManager::StopBGM()
{
    if (!audioInitialized) return;

    SDL_LockAudioDevice(deviceId);
    for (auto it = playingSounds.begin(); it != playingSounds.end(); )
    {
        if (it->isBGM)
        {
            it = playingSounds.erase(it);
        }
        else
        {
            ++it;
        }
    }
    SDL_UnlockAudioDevice(deviceId);
}

void CS230::SFXManager::StopAll()
{
    if (!audioInitialized) return;

    SDL_LockAudioDevice(deviceId);
    playingSounds.clear();
    SDL_UnlockAudioDevice(deviceId);
}

void CS230::SFXManager::SetSFXVolume(int volume)
{
    sfxVolume = volume;
}

void CS230::SFXManager::SetBGMVolume(int volume)
{
    bgmVolume = volume;
}

void CS230::SFXManager::SetMasterVolume(int volume)
{
    masterVolume = volume;
}

void CS230::SFXManager::Unload()
{
    StopAll();
    for (auto& pair : sounds)
    {
        if (pair.second.buffer)
        {
            SDL_free(pair.second.buffer);
        }
    }
    sounds.clear();
}

void CS230::SFXManager::AudioCallback(void* userdata, Uint8* stream, int len)
{
    SFXManager* manager = static_cast<SFXManager*>(userdata);
    SDL_memset(stream, 0, len); // Initialize stream with silence

    for (auto it = manager->playingSounds.begin(); it != manager->playingSounds.end(); )
    {
        PlayingSound& snd = *it;
        int trackVol = snd.isBGM ? manager->bgmVolume : manager->sfxVolume;

        // combine track volume, individual volume and master volume
        int finalVolume = (snd.volume * trackVol * manager->masterVolume) / (SDL_MIX_MAXVOLUME * SDL_MIX_MAXVOLUME);

        Uint32 remain = snd.audio->length - snd.position;
        Uint32 mixLen = (remain > static_cast<Uint32>(len)) ? len : remain;

        SDL_MixAudioFormat(stream, snd.audio->buffer + snd.position, manager->targetSpec.format, mixLen, finalVolume);
        snd.position += mixLen;

        if (snd.position >= snd.audio->length)
        {
            if (snd.loop)
            {
                snd.position = 0;
                Uint32 rest = len - mixLen;
                if (rest > 0 && snd.audio->length > 0)
                {
                    Uint32 mixLen2 = (snd.audio->length > rest) ? rest : snd.audio->length;
                    SDL_MixAudioFormat(stream + mixLen, snd.audio->buffer, manager->targetSpec.format, mixLen2, finalVolume);
                    snd.position += mixLen2;
                }
                ++it;
            }
            else
            {
                it = manager->playingSounds.erase(it);
            }
        }
        else
        {
            ++it;
        }
    }
}
