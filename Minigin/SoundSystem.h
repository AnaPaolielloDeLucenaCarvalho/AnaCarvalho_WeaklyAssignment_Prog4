#ifndef SOUND_SYSTEM_H
#define SOUND_SYSTEM_H

#include <string>

namespace dae
{
    using sound_id = unsigned short;

    class SoundSystem {
    public:
        virtual ~SoundSystem() = default;
        virtual void play(const sound_id id, const float volume) = 0;
        virtual void loadSound(const sound_id id, const std::string& filePath) = 0;
    };
}

#endif