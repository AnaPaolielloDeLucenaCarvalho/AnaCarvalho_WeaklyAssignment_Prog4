#ifndef ENEMY_SPAWNER_COMPONENT_H
#define ENEMY_SPAWNER_COMPONENT_H

#include "Component.h"
#include "GameObject.h"

#include <vector>

namespace dae
{
    class DiggerComponent;

    class EnemySpawnerComponent final : public Component
    {
    public:
        EnemySpawnerComponent(GameObject* owner, DiggerComponent* p1,
                              int maxTotal, int maxConcurrent);

        void Update(float deltaTime) override;

    private:
        DiggerComponent* m_p1;
        int m_MaxTotalEnemies;
        int m_MaxConcurrent;
        int m_TotalSpawned{ 0 };
        float m_SpawnTimer{ 0.0f }; // spawn 1 enemy immediately
        std::vector<GameObject*> m_SpawnedEnemies;
    };
}

#endif // ENEMY_SPAWNER_COMPONENT_H