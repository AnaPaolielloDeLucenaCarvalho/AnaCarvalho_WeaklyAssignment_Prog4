#ifndef MINIAUDIO_SOUND_SYSTEM_H
#define MINIAUDIO_SOUND_SYSTEM_H

#pragma once
#include "SoundSystem.h"
#include <memory>
#include <string>

namespace dae
{
    class MiniaudioSoundSystem final : public SoundSystem
    {
    public:
        MiniaudioSoundSystem();
        ~MiniaudioSoundSystem() override;

        void play(const sound_id id, const float volume) override;
        void loadSound(const sound_id id, const std::string& filePath) override;

        MiniaudioSoundSystem(const MiniaudioSoundSystem& other) = delete;
        MiniaudioSoundSystem(MiniaudioSoundSystem&& other) = delete;
        MiniaudioSoundSystem& operator=(const MiniaudioSoundSystem& other) = delete;
        MiniaudioSoundSystem& operator=(MiniaudioSoundSystem&& other) = delete;

    private:
        class MiniaudioSoundSystemImpl; // Pimpl
        std::unique_ptr<MiniaudioSoundSystemImpl> pImpl;
    };
}

#endif