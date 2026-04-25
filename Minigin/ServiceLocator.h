#ifndef SERVICE_LOCATOR_H
#define SERVICE_LOCATOR_H

#include "SoundSystem.h"
#include <memory>

namespace dae 
{
    class NullSoundSystem final : public SoundSystem 
    {
    public:
        void play(const sound_id, const float) override {}
        void loadSound(const sound_id, const std::string&) override {}
    };

    class ServiceLocator final 
    {
    public:
        static SoundSystem& get_sound_system() { return *_ss_instance; }
        static void register_sound_system(std::unique_ptr<SoundSystem>&& ss) 
        {
            _ss_instance = ss == nullptr ? std::make_unique<NullSoundSystem>() : std::move(ss);
        }
    private:
        static std::unique_ptr<SoundSystem> _ss_instance;
    };
}

#endif