#ifndef SOUND_SYSTEM_H
#define SOUND_SYSTEM_H

#include <string>

namespace dae
{
    using sound_id = unsigned short;

    class SoundSystem {
    public:
        virtual ~SoundSystem() = default;
        virtual void Play(const sound_id id, const float volume) = 0;
        virtual void LoadSound(const sound_id id, const std::string& filePath) = 0;

        virtual void ToggleMute() = 0;
    };
}

#endif