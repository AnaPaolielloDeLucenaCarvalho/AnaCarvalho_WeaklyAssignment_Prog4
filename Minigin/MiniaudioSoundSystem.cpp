#include "MiniaudioSoundSystem.h"

#define MA_NO_WASAPI
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <unordered_map>
#include <iostream>

namespace dae
{
    struct SoundRequest
    {
        sound_id id;
        float volume;
        bool isMusic{ false };
        bool loop{ false };
        bool isStop{ false };
        bool isPauseMusic{ false };
        bool isResumeMusic{ false };
        bool isSfxStop{ false };
        bool isSfx{ false };
    };

    class MiniaudioSoundSystem::MiniaudioSoundSystemImpl
    {
    public:
        MiniaudioSoundSystemImpl()
        {
            if (ma_engine_init(NULL, &m_AudioEngine) != MA_SUCCESS) 
            {
                std::cerr << "Failed to initialize miniaudio engine.\n";
            }

#ifndef __EMSCRIPTEN__
            m_Thread = std::jthread(&MiniaudioSoundSystemImpl::ProcessQueue, this);
#endif
        }

        ~MiniaudioSoundSystemImpl()
        {
#ifndef __EMSCRIPTEN__
            m_Quit = true;
            m_Condition.notify_one();
#endif
            if (m_MusicLoaded)
            {
                ma_sound_uninit(&m_MusicSound);
                m_MusicLoaded = false;
            }
            if (m_SfxLoaded)
            {
                ma_sound_uninit(&m_SfxSound);
                m_SfxLoaded = false;
            }
            ma_engine_uninit(&m_AudioEngine);
        }

        void Play(sound_id id, float volume)
        {
#ifndef __EMSCRIPTEN__
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_Queue.push({ id, volume, false, false, false });
            m_Condition.notify_one();
#else
            if (m_SoundPaths.contains(id)) 
            {
                ma_engine_play_sound(&m_AudioEngine, m_SoundPaths[id].c_str(), NULL);
            }
#endif
        }

        void PlayMusic(sound_id id, float volume, bool loop)
        {
#ifndef __EMSCRIPTEN__
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_Queue.push({ id, volume, true, loop, false });
            m_Condition.notify_one();
#else
            if (m_SoundPaths.contains(id))
            {
                if (m_MusicLoaded)
                {
                    ma_sound_uninit(&m_MusicSound);
                    m_MusicLoaded = false;
                }
                if (ma_sound_init_from_file(&m_AudioEngine, m_SoundPaths[id].c_str(),
                    MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, &m_MusicSound) == MA_SUCCESS)
                {
                    ma_sound_set_looping(&m_MusicSound, loop ? MA_TRUE : MA_FALSE);
                    ma_sound_set_volume(&m_MusicSound, volume);
                    ma_sound_start(&m_MusicSound);
                    m_MusicLoaded = true;
                }
            }
#endif
        }

        void PauseMusic()
        {
#ifndef __EMSCRIPTEN__
            std::lock_guard<std::mutex> lock(m_Mutex);
            SoundRequest req{};
            req.isPauseMusic = true;
            m_Queue.push(req);
            m_Condition.notify_one();
#else
            if (m_MusicLoaded)
            {
                ma_sound_stop(&m_MusicSound);
            }
#endif
        }

        void ResumeMusic()
        {
#ifndef __EMSCRIPTEN__
            std::lock_guard<std::mutex> lock(m_Mutex);
            SoundRequest req{};
            req.isResumeMusic = true;
            m_Queue.push(req);
            m_Condition.notify_one();
#else
            if (m_MusicLoaded)
            {
                ma_sound_start(&m_MusicSound);
            }
#endif
        }

        void StopMusic()
        {
#ifndef __EMSCRIPTEN__
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_Queue.push({ 0, 0.0f, false, false, true, false });
            m_Condition.notify_one();
#else
            if (m_MusicLoaded)
            {
                ma_sound_stop(&m_MusicSound);
            }
#endif
        }

        void PlaySfx(sound_id id, float volume)
        {
#ifndef __EMSCRIPTEN__
            std::lock_guard<std::mutex> lock(m_Mutex);
            SoundRequest req{};
            req.id     = id;
            req.volume = volume;
            req.isSfx  = true;
            m_Queue.push(req);
            m_Condition.notify_one();
#else
            if (m_SfxLoaded)
            {
                ma_sound_uninit(&m_SfxSound);
                m_SfxLoaded = false;
            }
            if (m_SoundPaths.contains(id))
            {
                if (ma_sound_init_from_file(&m_AudioEngine, m_SoundPaths[id].c_str(),
                    MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, &m_SfxSound) == MA_SUCCESS)
                {
                    ma_sound_set_volume(&m_SfxSound, volume);
                    ma_sound_start(&m_SfxSound);
                    m_SfxLoaded = true;
                }
            }
#endif
        }

        void StopSfx()
        {
#ifndef __EMSCRIPTEN__
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_Queue.push({ 0, 0.0f, false, false, false, true });
            m_Condition.notify_one();
#else
            if (m_SfxLoaded)
            {
                ma_sound_stop(&m_SfxSound);
            }
#endif
        }

        void LoadSound(sound_id id, const std::string& filePath)
        {
            m_SoundPaths[id] = filePath;
        }

        void ToggleMute()
        {
            m_Muted = !m_Muted;
            ma_engine_set_volume(&m_AudioEngine, m_Muted ? 0.0f : 1.0f);
        }

    private:
#ifndef __EMSCRIPTEN__
        void PlayMusicInternal(sound_id id, float volume, bool loop)
        {
            if (!m_SoundPaths.contains(id)) return;

            if (m_MusicLoaded)
            {
                ma_sound_stop(&m_MusicSound);
                ma_sound_uninit(&m_MusicSound);
                m_MusicLoaded = false;
            }

            if (ma_sound_init_from_file(&m_AudioEngine, m_SoundPaths[id].c_str(), MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, &m_MusicSound) == MA_SUCCESS)
            {
                ma_sound_set_looping(&m_MusicSound, loop ? MA_TRUE : MA_FALSE);
                ma_sound_set_volume(&m_MusicSound, volume);
                ma_sound_start(&m_MusicSound);
                m_MusicLoaded = true;
            }
            else
            {
                std::cerr << "[MiniaudioSoundSystem] Failed to load music track: " << m_SoundPaths[id] << "\n";
            }
        }

        void StopMusicInternal()
        {
            if (m_MusicLoaded)
            {
                ma_sound_stop(&m_MusicSound);
            }
        }

        void PauseMusicInternal()
        {
            if (m_MusicLoaded)
            {
                ma_sound_stop(&m_MusicSound);
            }
        }

        void ResumeMusicInternal()
        {
            if (m_MusicLoaded)
            {
                ma_sound_start(&m_MusicSound);
            }
        }

        void PlaySfxInternal(sound_id id, float volume)
        {
            if (!m_SoundPaths.contains(id)) return;

            if (m_SfxLoaded)
            {
                ma_sound_stop(&m_SfxSound);
                ma_sound_uninit(&m_SfxSound);
                m_SfxLoaded = false;
            }

            if (ma_sound_init_from_file(&m_AudioEngine, m_SoundPaths[id].c_str(),
                MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, &m_SfxSound) == MA_SUCCESS)
            {
                ma_sound_set_volume(&m_SfxSound, volume);
                ma_sound_start(&m_SfxSound);
                m_SfxLoaded = true;
            }
            else
            {
                std::cerr << "[MiniaudioSoundSystem] Failed to load sfx track: " << m_SoundPaths[id] << "\n";
            }
        }

        void StopSfxInternal()
        {
            if (m_SfxLoaded)
            {
                ma_sound_stop(&m_SfxSound);
            }
        }

        void ProcessQueue()
        {
            while (true)
            {
                std::unique_lock<std::mutex> lock(m_Mutex);
                m_Condition.wait(lock, [this]() { return !m_Queue.empty() || m_Quit; });

                if (m_Quit && m_Queue.empty()) break;

                SoundRequest request = m_Queue.front();
                m_Queue.pop();
                lock.unlock();

                if (request.isStop)
                {
                    StopMusicInternal();
                }
                else if (request.isPauseMusic)
                {
                    PauseMusicInternal();
                }
                else if (request.isResumeMusic)
                {
                    ResumeMusicInternal();
                }
                else if (request.isSfxStop)
                {
                    StopSfxInternal();
                }
                else if (request.isMusic)
                {
                    PlayMusicInternal(request.id, request.volume, request.loop);
                }
                else if (request.isSfx)
                {
                    PlaySfxInternal(request.id, request.volume);
                }
                else if (m_SoundPaths.contains(request.id))
                {
                    ma_engine_play_sound(&m_AudioEngine, m_SoundPaths[request.id].c_str(), NULL);
                }
            }
        }

        std::jthread m_Thread;
        std::mutex m_Mutex;
        std::condition_variable m_Condition;
        std::queue<SoundRequest> m_Queue;
        bool m_Quit{ false };
#endif

        ma_engine m_AudioEngine;
        ma_sound  m_MusicSound;
        bool      m_MusicLoaded{ false };
        ma_sound  m_SfxSound;
        bool      m_SfxLoaded{ false };
        std::unordered_map<sound_id, std::string> m_SoundPaths;
        bool m_Muted{ false };
    };

    MiniaudioSoundSystem::MiniaudioSoundSystem() : pImpl(std::make_unique<MiniaudioSoundSystemImpl>()) {}
    MiniaudioSoundSystem::~MiniaudioSoundSystem() = default;
    void MiniaudioSoundSystem::Play(const sound_id id, const float volume) { pImpl->Play(id, volume); }
    void MiniaudioSoundSystem::PlayMusic(const sound_id id, const float volume, const bool loop) { pImpl->PlayMusic(id, volume, loop); }
    void MiniaudioSoundSystem::PauseMusic() { pImpl->PauseMusic(); }
    void MiniaudioSoundSystem::ResumeMusic() { pImpl->ResumeMusic(); }
    void MiniaudioSoundSystem::StopMusic() { pImpl->StopMusic(); }
    void MiniaudioSoundSystem::PlaySfx(const sound_id id, const float volume) { pImpl->PlaySfx(id, volume); }
    void MiniaudioSoundSystem::StopSfx() { pImpl->StopSfx(); }
    void MiniaudioSoundSystem::LoadSound(const sound_id id, const std::string& filePath) { pImpl->LoadSound(id, filePath); }
    void MiniaudioSoundSystem::ToggleMute() { pImpl->ToggleMute(); }
}