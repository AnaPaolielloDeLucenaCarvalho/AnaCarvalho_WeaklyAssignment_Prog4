#ifndef LEVEL_TRANSITION_MANAGER_H
#define LEVEL_TRANSITION_MANAGER_H

#include "Component.h"
#include "Observer.h"
#include "GameObject.h"

#include <vector>
#include <cstdint>

namespace dae
{
    class Scene;
    class DiggerComponent;

    class LevelTransitionManager final : public Component, public Observer
    {
    public:
        LevelTransitionManager(GameObject* owner, Scene* scene, DiggerComponent* p1, DiggerComponent* p2);

        void Update(float deltaTime) override;
        void OnNotify(EventId eventId, int value) override;

        // Called once by Main.cpp for the initial level load.
        void LoadLevel(int levelIndex);

    private:
        Scene* m_pScene;
        DiggerComponent* m_p1;
        DiggerComponent* m_p2;
        std::vector<GameObject*> m_VisualDirt;
        int m_CurrentLevelIndex{ 0 };
        uint64_t m_LastLoadTime{ 0 };

        // 'B' tile position — set during LoadLevel, used by SpawnCherry()
        float m_CherrySpawnX{ 0.0f };
        float m_CherrySpawnY{ 0.0f };
        bool m_CherrySpawned{ false };

        void SpawnCherry();

        // bonus - map flicker
        bool m_BonusMapActive{ false };
        bool m_BonusFlickerPhase{ false }; // true during - first 5s so it flickers
        float m_BonusFlickerTimer{ 0.0f };
        float m_BonusFlickerInterval{ 0.0f };
        bool m_BonusLightOn{ false }; // which phase of the flicker we are in

        void ApplyDirtBrightness(bool enable);
    };
}

#endif // LEVEL_TRANSITION_MANAGER_H
