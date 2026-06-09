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
        LevelTransitionManager(GameObject* owner, Scene* scene,
                               DiggerComponent* p1, DiggerComponent* p2);

        void Update(float deltaTime)              override;
        void OnNotify(EventId eventId, int value) override;

        void LoadLevel(int levelIndex);

    private:
        Scene*                   m_pScene;
        DiggerComponent*         m_p1;
        DiggerComponent*         m_p2;
        std::vector<GameObject*> m_VisualDirt;
        int                      m_CurrentLevelIndex{ 0 };
        uint64_t                 m_LastLoadTime{ 0 };
    };
}

#endif // LEVEL_TRANSITION_MANAGER_H
