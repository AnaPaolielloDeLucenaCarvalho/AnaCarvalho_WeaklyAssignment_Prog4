#ifndef LOGGING_SOUND_SYSTEM_H
#define LOGGING_SOUND_SYSTEM_H

#include "SoundSystem.h"
#include <memory>
#include <string>

namespace dae
{
    class LoggingSoundSystem final : public SoundSystem
    {
    public:
        LoggingSoundSystem(std::unique_ptr<SoundSystem>&& ss);
        ~LoggingSoundSystem() override = default;

        void Play(const sound_id id, const float volume) override;
        void LoadSound(const sound_id id, const std::string& filePath) override;
        void ToggleMute() override;

    private:
        std::unique_ptr<SoundSystem> _realSS;
    };
}
#endif