#ifndef LOGGING_SOUND_SYSTEM_H
#define LOGGING_SOUND_SYSTEM_H

#include "SoundSystem.h"
#include <memory>
#include <iostream>

namespace dae
{
    class LoggingSoundSystem final : public SoundSystem
    {
    public:
        LoggingSoundSystem(std::unique_ptr<SoundSystem>&& ss) : _realSS(std::move(ss)) {}

        void Play(const sound_id id, const float volume) override
        {
            std::cout << "[Sound Logger] Playing sound ID: " << id << " at volume: " << volume << "\n";
            _realSS->Play(id, volume);
        }

        void LoadSound(const sound_id id, const std::string& filePath) override
        {
            std::cout << "[Sound Logger] Loading sound ID: " << id << " from " << filePath << "\n";
            _realSS->LoadSound(id, filePath);
        }

        void ToggleMute() override
        {
            std::cout << "[Sound Logger] Toggling Audio Mute State\n";
            _realSS->ToggleMute();
        }

    private:
            std::unique_ptr<SoundSystem> _realSS;
    };
}

#endif 
