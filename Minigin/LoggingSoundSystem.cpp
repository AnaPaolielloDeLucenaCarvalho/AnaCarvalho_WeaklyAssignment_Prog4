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