#ifndef LOGGING_SOUND_SYSTEM_H
#define LOGGING_SOUND_SYSTEM_H

#include "SoundSystem.h"
#include <memory>
#include <string>

// DESIGN PATTERN - Decorator Pattern (Proxy)
// This class wraps the real SoundSystem. It intercepts every audio call, prints a debug message to  the console, and then forwards the call to the real audio engine. It allows us to seamlessly toggle debug logging on and off without bloating the actual Miniaudio code with std::cout statements.

namespace dae
{
    class LoggingSoundSystem final : public SoundSystem
    {
    public:
        // DESIGN DECISION - Strict Ownership (RAII)
        // Passing std::unique_ptr transfers absolute ownership of the real sound system to the logger.
        LoggingSoundSystem(std::unique_ptr<SoundSystem>&& ss);
        ~LoggingSoundSystem() override = default;

        void Play(const sound_id id, const float volume) override;
        void PlayMusic(const sound_id id, const float volume, const bool loop) override;
        void PauseMusic() override;
        void ResumeMusic() override;
        void StopMusic() override;
        void PlaySfx(const sound_id id, const float volume) override;
        void StopSfx() override;
        void LoadSound(const sound_id id, const std::string& filePath) override;
        void ToggleMute() override;

    private:
        std::unique_ptr<SoundSystem> m_pRealSS;
    };
}
#endif