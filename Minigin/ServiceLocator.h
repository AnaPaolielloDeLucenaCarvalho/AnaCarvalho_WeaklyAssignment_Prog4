#ifndef SERVICE_LOCATOR_H
#define SERVICE_LOCATOR_H

#include "SoundSystem.h"
#include <memory>

namespace dae 
{
    class NullSoundSystem final : public SoundSystem 
    {
    public:
        void Play(const sound_id, const float) override {}
        void PlayMusic(const sound_id, const float, const bool) override {}
        void PauseMusic() override {}
        void ResumeMusic() override {}
        void StopMusic() override {}
        void PlaySfx(const sound_id, const float) override {}
        void StopSfx() override {}
        void LoadSound(const sound_id, const std::string&) override {}

        void ToggleMute() override {}
    };

    class ServiceLocator final 
    {
    public:
        static SoundSystem& GetSoundSystem() { return *_ssInstance; }
        static void RegisterSoundSystem(std::unique_ptr<SoundSystem>&& ss) 
        {
            _ssInstance = ss == nullptr ? std::make_unique<NullSoundSystem>() : std::move(ss);
        }
    private:
        static std::unique_ptr<SoundSystem> _ssInstance;
    };
}

#endif