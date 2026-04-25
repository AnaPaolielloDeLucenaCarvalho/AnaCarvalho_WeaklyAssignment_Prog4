#include "MiniaudioSoundSystem.h"

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

            // create a thread if we are NOT in the web
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
            // clean up miniaudio
            ma_engine_uninit(&m_AudioEngine);
        }

        void Play(sound_id id, float volume)
        {
#ifndef __EMSCRIPTEN__
            // windows -
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_Queue.push({ id, volume });
            m_Condition.notify_one();
#else
            // web - 
            if (m_SoundPaths.contains(id))
            {
                ma_engine_play_sound(&m_AudioEngine, m_SoundPaths[id].c_str(), NULL);
            }
#endif
        }

        void LoadSound(sound_id id, const std::string& filePath)
        {
            m_SoundPaths[id] = filePath;
        }

    private:

#ifndef __EMSCRIPTEN__
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

                // play the sound
                if (m_SoundPaths.contains(request.id))
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
        std::unordered_map<sound_id, std::string> m_SoundPaths;
    };

    MiniaudioSoundSystem::MiniaudioSoundSystem() : pImpl(std::make_unique<MiniaudioSoundSystemImpl>()) {}
    MiniaudioSoundSystem::~MiniaudioSoundSystem() = default;
    void MiniaudioSoundSystem::play(const sound_id id, const float volume) { pImpl->Play(id, volume); }
    void MiniaudioSoundSystem::loadSound(const sound_id id, const std::string& filePath) { pImpl->LoadSound(id, filePath); }
}

// Note for the teacher: i know i have memory leaks in this implementation, but i from my looking they come from the miniaudio that im using for the sound system, and i dont know how to fix them, it seems to me they come from MMDevApi.dll/AudioSes.dll/uxtheme.dll but i could be wrong, if you have any tips on how to fix this please let me know and im sorry for the inconvenience