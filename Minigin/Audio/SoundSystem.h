#ifndef SOUND_SYSTEM_H
#define SOUND_SYSTEM_H

#include <string>

// DESIGN PATTERN - Interface (Service Locator Target)
// This is a pure abstract interface. By programming against this interface rather than a concrete  class (like Miniaudio), the rest of the game engine is completely decoupled from the audio backend.  We could swap out Miniaudio for FMOD or SDL_Mixer tomorrow, and the Digger component wouldn't need a single line of code changed.

namespace dae
{
    using sound_id = unsigned short;

    class SoundSystem {
    public:
        virtual ~SoundSystem() = default;


        virtual void Play(const sound_id id, const float volume) = 0;
        virtual void PlayMusic(const sound_id id, const float volume, const bool loop) = 0;
        virtual void PauseMusic() = 0;
        virtual void ResumeMusic() = 0;
        virtual void StopMusic() = 0;
        virtual void PlaySfx(const sound_id id, const float volume) = 0;
        virtual void StopSfx() = 0;
        virtual void LoadSound(const sound_id id, const std::string& filePath) = 0;

        virtual void ToggleMute() = 0;

        SoundSystem(const SoundSystem& other) = delete;
        SoundSystem(SoundSystem&& other) = delete;
        SoundSystem& operator=(const SoundSystem& other) = delete;
        SoundSystem& operator=(SoundSystem&& other) = delete;

    protected:
        SoundSystem() = default;
    };
}

#endif