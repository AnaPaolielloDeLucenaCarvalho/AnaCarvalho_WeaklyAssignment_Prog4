#ifndef LEVEL_TRANSITION_MANAGER_H
#define LEVEL_TRANSITION_MANAGER_H

#include "Component.h"
#include "Observer.h"
#include "GameObject.h"

#include <vector>
#include <cstdint>

// DESIGN PATTERN - Observer Pattern & Component Pattern
// The central director for building and clearing the grid. Uses the Observer pattern to listen for "LoadNextLevel" or "EnemyThresholdReached" events, allowing to spawn the cherry or change level without the player or enemies needing to know about each other

namespace dae
{
    class Scene;
    class DiggerComponent;
    class LivesSpriteDisplayComponent;

    class LevelTransitionManager final : public Component, public Observer
    {
    public:
        LevelTransitionManager(GameObject* owner, Scene* scene, DiggerComponent* p1, DiggerComponent* p2, GameObject* p2Label = nullptr, GameObject* p2Lives = nullptr, GameObject* scoreUI1 = nullptr, LivesSpriteDisplayComponent* p2LivesUI = nullptr);

        void Render() const override {}
        void Update(float deltaTime) override;
        void OnNotify(EventId eventId, int value) override;

        // Called once by Main.cpp for the initial level load.
        void LoadLevel(int levelIndex);

    private:
        Scene* m_pScene;
        DiggerComponent* m_p1;
        DiggerComponent* m_p2;
        GameObject* m_p2Label;
        GameObject* m_p2Lives;
        GameObject* m_pScoreUI1;
        LivesSpriteDisplayComponent* m_p2LivesUI;
        std::vector<GameObject*> m_visualDirt;
        int m_currentLevelIndex{ 0 };
        uint64_t m_lastLoadTime{ 0 };
        bool m_hasLoadedFirstLevel{ false };

        std::vector<GameObject*> m_miscEntities;

        // 'B' tile position — set during LoadLevel, used by SpawnCherry()
        float m_cherrySpawnX{ 0.0f };
        float m_cherrySpawnY{ 0.0f };
        bool m_cherrySpawned{ false };

        void SpawnCherry();

        // bonus - map flicker
        bool m_bonusMapActive{ false };
        bool m_bonusFlickerPhase{ false }; // true during - first 5s so it flickers
        float m_bonusFlickerTimer{ 0.0f };
        float m_bonusFlickerInterval{ 0.0f };
        bool m_bonusLightOn{ false }; // which phase of the flicker we are in

        void ApplyDirtBrightness(bool enable);
    };
}

#endif // LEVEL_TRANSITION_MANAGER_H
