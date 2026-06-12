#include "ServiceLocator.h"

namespace dae
{
    void NullSoundSystem::Play(const sound_id, const float) {}
    void NullSoundSystem::PlayMusic(const sound_id, const float, const bool) {}
    void NullSoundSystem::PauseMusic() {}
    void NullSoundSystem::ResumeMusic() {}
    void NullSoundSystem::StopMusic() {}
    void NullSoundSystem::PlaySfx(const sound_id, const float) {}
    void NullSoundSystem::StopSfx() {}
    void NullSoundSystem::LoadSound(const sound_id, const std::string&) {}
    void NullSoundSystem::ToggleMute() {}

    std::unique_ptr<SoundSystem> ServiceLocator::s_ssInstance{ std::make_unique<NullSoundSystem>() };

    SoundSystem& ServiceLocator::GetSoundSystem() 
    { 
        return *s_ssInstance; 
    }

    void ServiceLocator::RegisterSoundSystem(std::unique_ptr<SoundSystem>&& ss) 
    {
        s_ssInstance = ss == nullptr ? std::make_unique<NullSoundSystem>() : std::move(ss);
    }
}