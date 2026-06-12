#include "LoggingSoundSystem.h"
#include <iostream>

namespace dae
{
    LoggingSoundSystem::LoggingSoundSystem(std::unique_ptr<SoundSystem>&& ss)
        : _realSS(std::move(ss))
    {
    }

    void LoggingSoundSystem::Play(const sound_id id, const float volume)
    {
        std::cout << "[Sound Logger] Playing sound ID: " << id << " at volume: " << volume << "\n";
        _realSS->Play(id, volume);
    }

    void LoggingSoundSystem::PlayMusic(const sound_id id, const float volume, const bool loop)
    {
        std::cout << "[Sound Logger] Playing music ID: " << id << " at volume: " << volume << " loop: " << loop << "\n";
        _realSS->PlayMusic(id, volume, loop);
    }

    void LoggingSoundSystem::PauseMusic()
    {
        std::cout << "[Sound Logger] Pausing music\n";
        _realSS->PauseMusic();
    }

    void LoggingSoundSystem::ResumeMusic()
    {
        std::cout << "[Sound Logger] Resuming music\n";
        _realSS->ResumeMusic();
    }

    void LoggingSoundSystem::StopMusic()
    {
        std::cout << "[Sound Logger] Stopping music\n";
        _realSS->StopMusic();
    }

    void LoggingSoundSystem::PlaySfx(const sound_id id, const float volume)
    {
        std::cout << "[Sound Logger] Playing sfx ID: " << id << " at volume: " << volume << "\n";
        _realSS->PlaySfx(id, volume);
    }

    void LoggingSoundSystem::StopSfx()
    {
        std::cout << "[Sound Logger] Stopping sfx\n";
        _realSS->StopSfx();
    }

    void LoggingSoundSystem::LoadSound(const sound_id id, const std::string& filePath)
    {
        std::cout << "[Sound Logger] Loading sound ID: " << id << " from " << filePath << "\n";
        _realSS->LoadSound(id, filePath);
    }

    void LoggingSoundSystem::ToggleMute()
    {
        std::cout << "[Sound Logger] Toggling Audio Mute State\n";
        _realSS->ToggleMute();
    }
}