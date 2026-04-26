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
        LoggingSoundSystem(std::unique_ptr<SoundSystem>&& ss) : _real_ss(std::move(ss)) {}

        void play(const sound_id id, const float volume) override
        {
            std::cout << "[Sound Logger] Playing sound ID: " << id << " at volume: " << volume << "\n";
            _real_ss->play(id, volume);
        }

        void loadSound(const sound_id id, const std::string& filePath) override
        {
            std::cout << "[Sound Logger] Loading sound ID: " << id << " from " << filePath << "\n";
            _real_ss->loadSound(id, filePath);
        }

        void ToggleMute() override
        {
            std::cout << "[Sound Logger] Toggling Audio Mute State\n";
            _real_ss->ToggleMute();
        }

    private:
            std::unique_ptr<SoundSystem> _real_ss;
    };
}

#endif 
