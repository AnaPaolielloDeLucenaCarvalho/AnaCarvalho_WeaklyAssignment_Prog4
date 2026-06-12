#include "LoggingSoundSystem.h"
#include <iostream>

namespace dae
{
    LoggingSoundSystem::LoggingSoundSystem(std::unique_ptr<SoundSystem>&& ss)
        : m_pRealSS(std::move(ss))
    {
    }

    void LoggingSoundSystem::Play(const sound_id id, const float volume)
    {
        std::cout << "[Sound Logger] Playing sound ID: " << id << " at volume: " << volume << "\n";
        m_pRealSS->Play(id, volume);
    }

    void LoggingSoundSystem::PlayMusic(const sound_id id, const float volume, const bool loop)
    {
        std::cout << "[Sound Logger] Playing music ID: " << id << " at volume: " << volume << " loop: " << loop << "\n";
        m_pRealSS->PlayMusic(id, volume, loop);
    }

    void LoggingSoundSystem::PauseMusic()
    {
        std::cout << "[Sound Logger] Pausing music\n";
        m_pRealSS->PauseMusic();
    }

    void LoggingSoundSystem::ResumeMusic()
    {
        std::cout << "[Sound Logger] Resuming music\n";
        m_pRealSS->ResumeMusic();
    }

    void LoggingSoundSystem::StopMusic()
    {
        std::cout << "[Sound Logger] Stopping music\n";
        m_pRealSS->StopMusic();
    }

    void LoggingSoundSystem::PlaySfx(const sound_id id, const float volume)
    {
        std::cout << "[Sound Logger] Playing sfx ID: " << id << " at volume: " << volume << "\n";
        m_pRealSS->PlaySfx(id, volume);
    }

    void LoggingSoundSystem::StopSfx()
    {
        std::cout << "[Sound Logger] Stopping sfx\n";
        m_pRealSS->StopSfx();
    }

    void LoggingSoundSystem::LoadSound(const sound_id id, const std::string& filePath)
    {
        std::cout << "[Sound Logger] Loading sound ID: " << id << " from " << filePath << "\n";
        m_pRealSS->LoadSound(id, filePath);
    }

    void LoggingSoundSystem::ToggleMute()
    {
        std::cout << "[Sound Logger] Toggling Audio Mute State\n";
        m_pRealSS->ToggleMute();
    }
}