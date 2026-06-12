#ifndef MINIAUDIO_SOUND_SYSTEM_H
#define MINIAUDIO_SOUND_SYSTEM_H

#include "SoundSystem.h"
#include <memory>
#include <string>

// DESIGN PATTERN - Pimpl Idiom (Pointer to Implementation)
// This is a crucial architectural optimization. By hiding the actual Miniaudio data structures inside an opaque 'pImpl' pointer, we avoid putting `#include "miniaudio.h"` in this header file.  If it was in the header, every single file that included SoundSystem.h would be forced to compile the massive Miniaudio library, which would completely destroy our C++ compilation times!

namespace dae
{
    class MiniaudioSoundSystem final : public SoundSystem
    {
    public:
        MiniaudioSoundSystem();
        ~MiniaudioSoundSystem() override;

        void Play(const sound_id id, const float volume) override;
        void PlayMusic(const sound_id id, const float volume, const bool loop) override;
        void PauseMusic() override;
        void ResumeMusic() override;
        void StopMusic() override;
        void PlaySfx(const sound_id id, const float volume) override;
        void StopSfx() override;
        void LoadSound(const sound_id id, const std::string& filePath) override;

        void ToggleMute() override;

        MiniaudioSoundSystem(const MiniaudioSoundSystem& other) = delete;
        MiniaudioSoundSystem(MiniaudioSoundSystem&& other) = delete;
        MiniaudioSoundSystem& operator=(const MiniaudioSoundSystem& other) = delete;
        MiniaudioSoundSystem& operator=(MiniaudioSoundSystem&& other) = delete;

    private:
        // DESIGN DECISION - RAII
        // Using a unique_ptr guarantees the internal audio engine is safely destroyed when the game closes.
        class MiniaudioSoundSystemImpl; // Pimpl
        std::unique_ptr<MiniaudioSoundSystemImpl> pImpl;
    };
}

#endif