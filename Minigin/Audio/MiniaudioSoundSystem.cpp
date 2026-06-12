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
            if (ma_engine_init(NULL, &m_audioEngine) != MA_SUCCESS) 
            {
                std::cerr << "Failed to initialize miniaudio engine.\n";
            }

#ifndef __EMSCRIPTEN__
            m_thread = std::jthread(&MiniaudioSoundSystemImpl::ProcessQueue, this);
#endif
        }

        ~MiniaudioSoundSystemImpl()
        {
#ifndef __EMSCRIPTEN__
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_quit = true;
                lock.unlock();
                m_condition.notify_one();
            }
#endif
            if (m_musicLoaded)
            {
                ma_sound_uninit(&m_musicSound);
                m_musicLoaded = false;
            }
            if (m_sfxLoaded)
            {
                ma_sound_uninit(&m_sfxSound);
                m_sfxLoaded = false;
            }
            ma_engine_uninit(&m_audioEngine);
        }

        void Play(sound_id id, float volume)
        {
#ifndef __EMSCRIPTEN__
            std::unique_lock<std::mutex> lock(m_mutex);
            SoundRequest req{};
            req.id = id;
            req.volume = volume;
            m_queue.push(req);
            lock.unlock();
            m_condition.notify_one();
#else
            if (m_soundPaths.contains(id)) 
            {
                ma_engine_play_sound(&m_audioEngine, m_soundPaths[id].c_str(), NULL);
            }
#endif
        }

        void PlayMusic(sound_id id, float volume, bool loop)
        {
#ifndef __EMSCRIPTEN__
            std::unique_lock<std::mutex> lock(m_mutex);
            SoundRequest req{};
            req.id = id;
            req.volume = volume;
            req.isMusic = true;
            req.loop = loop;
            m_queue.push(req);
            lock.unlock();
            m_condition.notify_one();
#else
            if (m_soundPaths.contains(id))
            {
                if (m_musicLoaded)
                {
                    ma_sound_uninit(&m_musicSound);
                    m_musicLoaded = false;
                }
                if (ma_sound_init_from_file(&m_audioEngine, m_soundPaths[id].c_str(),
                    MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, &m_musicSound) == MA_SUCCESS)
                {
                    ma_sound_set_looping(&m_musicSound, loop ? MA_TRUE : MA_FALSE);
                    ma_sound_set_volume(&m_musicSound, volume);
                    ma_sound_start(&m_musicSound);
                    m_musicLoaded = true;
                }
            }
#endif
        }

        void PauseMusic()
        {
#ifndef __EMSCRIPTEN__
            std::unique_lock<std::mutex> lock(m_mutex);
            SoundRequest req{};
            req.isPauseMusic = true;
            m_queue.push(req);
            lock.unlock();
            m_condition.notify_one();
#else
            if (m_musicLoaded)
            {
                ma_sound_stop(&m_musicSound);
            }
#endif
        }

        void ResumeMusic()
        {
#ifndef __EMSCRIPTEN__
            std::unique_lock<std::mutex> lock(m_mutex);
            SoundRequest req{};
            req.isResumeMusic = true;
            m_queue.push(req);
            lock.unlock();
            m_condition.notify_one();
#else
            if (m_musicLoaded)
            {
                ma_sound_start(&m_musicSound);
            }
#endif
        }

        void StopMusic()
        {
#ifndef __EMSCRIPTEN__
            std::unique_lock<std::mutex> lock(m_mutex);
            SoundRequest req{};
            req.isStop = true;
            m_queue.push(req);
            lock.unlock();
            m_condition.notify_one();
#else
            if (m_musicLoaded)
            {
                ma_sound_stop(&m_musicSound);
            }
#endif
        }

        void PlaySfx(sound_id id, float volume)
        {
#ifndef __EMSCRIPTEN__
            std::unique_lock<std::mutex> lock(m_mutex);
            SoundRequest req{};
            req.id     = id;
            req.volume = volume;
            req.isSfx  = true;
            m_queue.push(req);
            lock.unlock();
            m_condition.notify_one();
#else
            if (m_sfxLoaded)
            {
                ma_sound_uninit(&m_sfxSound);
                m_sfxLoaded = false;
            }
            if (m_soundPaths.contains(id))
            {
                if (ma_sound_init_from_file(&m_audioEngine, m_soundPaths[id].c_str(),
                    MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, &m_sfxSound) == MA_SUCCESS)
                {
                    ma_sound_set_volume(&m_sfxSound, volume);
                    ma_sound_start(&m_sfxSound);
                    m_sfxLoaded = true;
                }
            }
#endif
        }

        void StopSfx()
        {
#ifndef __EMSCRIPTEN__
            std::unique_lock<std::mutex> lock(m_mutex);
            SoundRequest req{};
            req.isSfxStop = true;
            m_queue.push(req);
            lock.unlock();
            m_condition.notify_one();
#else
            if (m_sfxLoaded)
            {
                ma_sound_stop(&m_sfxSound);
            }
#endif
        }

        void LoadSound(sound_id id, const std::string& filePath)
        {
            m_soundPaths[id] = filePath;
        }

        void ToggleMute()
        {
            m_muted = !m_muted;
            ma_engine_set_volume(&m_audioEngine, m_muted ? 0.0f : 1.0f);
        }

    private:
#ifndef __EMSCRIPTEN__
        void PlayMusicInternal(sound_id id, float volume, bool loop)
        {
            if (!m_soundPaths.contains(id)) return;

            if (m_musicLoaded)
            {
                ma_sound_stop(&m_musicSound);
                ma_sound_uninit(&m_musicSound);
                m_musicLoaded = false;
            }

            if (ma_sound_init_from_file(&m_audioEngine, m_soundPaths[id].c_str(), MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, &m_musicSound) == MA_SUCCESS)
            {
                ma_sound_set_looping(&m_musicSound, loop ? MA_TRUE : MA_FALSE);
                ma_sound_set_volume(&m_musicSound, volume);
                ma_sound_start(&m_musicSound);
                m_musicLoaded = true;
            }
            else
            {
                std::cerr << "[MiniaudioSoundSystem] Failed to load music track: " << m_soundPaths[id] << "\n";
            }
        }

        void StopMusicInternal()
        {
            if (m_musicLoaded)
            {
                ma_sound_stop(&m_musicSound);
            }
        }

        void PauseMusicInternal()
        {
            if (m_musicLoaded)
            {
                ma_sound_stop(&m_musicSound);
            }
        }

        void ResumeMusicInternal()
        {
            if (m_musicLoaded)
            {
                ma_sound_start(&m_musicSound);
            }
        }

        void PlaySfxInternal(sound_id id, float volume)
        {
            if (!m_soundPaths.contains(id)) return;

            if (m_sfxLoaded)
            {
                ma_sound_stop(&m_sfxSound);
                ma_sound_uninit(&m_sfxSound);
                m_sfxLoaded = false;
            }

            if (ma_sound_init_from_file(&m_audioEngine, m_soundPaths[id].c_str(),
                MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, &m_sfxSound) == MA_SUCCESS)
            {
                ma_sound_set_volume(&m_sfxSound, volume);
                ma_sound_start(&m_sfxSound);
                m_sfxLoaded = true;
            }
            else
            {
                std::cerr << "[MiniaudioSoundSystem] Failed to load sfx track: " << m_soundPaths[id] << "\n";
            }
        }

        void StopSfxInternal()
        {
            if (m_sfxLoaded)
            {
                ma_sound_stop(&m_sfxSound);
            }
        }

        void ProcessQueue()
        {
            while (true)
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_condition.wait(lock, [this]() { return !m_queue.empty() || m_quit; });

                if (m_quit && m_queue.empty()) break;

                SoundRequest request = m_queue.front();
                m_queue.pop();
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
                else if (m_soundPaths.contains(request.id))
                {
                    ma_engine_play_sound(&m_audioEngine, m_soundPaths[request.id].c_str(), NULL);
                }
            }
        }

        std::jthread m_thread;
        std::mutex m_mutex;
        std::condition_variable m_condition;
        std::queue<SoundRequest> m_queue;
        bool m_quit{ false };
#endif

        ma_engine m_audioEngine;
        ma_sound  m_musicSound;
        bool      m_musicLoaded{ false };
        ma_sound  m_sfxSound;
        bool      m_sfxLoaded{ false };
        std::unordered_map<sound_id, std::string> m_soundPaths;
        bool m_muted{ false };
    };

    MiniaudioSoundSystem::MiniaudioSoundSystem() : m_pImpl(std::make_unique<MiniaudioSoundSystemImpl>()) {}
    MiniaudioSoundSystem::~MiniaudioSoundSystem() = default;
    void MiniaudioSoundSystem::Play(const sound_id id, const float volume) { m_pImpl->Play(id, volume); }
    void MiniaudioSoundSystem::PlayMusic(const sound_id id, const float volume, const bool loop) { m_pImpl->PlayMusic(id, volume, loop); }
    void MiniaudioSoundSystem::PauseMusic() { m_pImpl->PauseMusic(); }
    void MiniaudioSoundSystem::ResumeMusic() { m_pImpl->ResumeMusic(); }
    void MiniaudioSoundSystem::StopMusic() { m_pImpl->StopMusic(); }
    void MiniaudioSoundSystem::PlaySfx(const sound_id id, const float volume) { m_pImpl->PlaySfx(id, volume); }
    void MiniaudioSoundSystem::StopSfx() { m_pImpl->StopSfx(); }
    void MiniaudioSoundSystem::LoadSound(const sound_id id, const std::string& filePath) { m_pImpl->LoadSound(id, filePath); }
    void MiniaudioSoundSystem::ToggleMute() { m_pImpl->ToggleMute(); }
}