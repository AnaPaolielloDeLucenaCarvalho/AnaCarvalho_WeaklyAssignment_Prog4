#ifndef ENEMY_SPAWNER_COMPONENT_H
#define ENEMY_SPAWNER_COMPONENT_H

#include "Component.h"
#include "Subject.h"
#include "GameObject.h"

#include <vector>

namespace dae
{
    class DiggerComponent;
    class Observer;

    class EnemySpawnerComponent final : public Component
    {
    public:
        EnemySpawnerComponent(GameObject* owner, DiggerComponent* p1, DiggerComponent* p2, int maxTotal, int maxConcurrent);

        void Update(float deltaTime) override;

        void AddObserver(Observer* obs);

    private:
        DiggerComponent* m_p1;
        DiggerComponent* m_p2;
        int m_MaxTotalEnemies;
        int m_MaxConcurrent;
        int m_TotalSpawned{ 0 };
        float m_SpawnTimer{ 0.0f }; // spawn 1 enemy immediately
        bool m_ThresholdNotified{ false }; // fire only once per level
        std::vector<GameObject*> m_SpawnedEnemies;
        Subject m_Subject;
    };
}

#endif // ENEMY_SPAWNER_COMPONENT_H