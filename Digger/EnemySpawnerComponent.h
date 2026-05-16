#ifndef ENEMY_SPAWNER_COMPONENT_H
#define ENEMY_SPAWNER_COMPONENT_H

#include "Component.h"
#include "GameObject.h"
#include "SceneManager.h"
#include "Scene.h"
#include "DiggerComponent.h"
#include "EnemyComponent.h"
#include "RenderComponent.h"
#include <vector>

namespace dae
{
    class EnemySpawnerComponent : public Component
    {
    public:
        EnemySpawnerComponent(GameObject* owner, DiggerComponent* p1, int maxTotal, int maxConcurrent)
            : Component(owner)
            , m_p1(p1)
            , m_MaxTotalEnemies(maxTotal)
            , m_MaxConcurrent(maxConcurrent) 
        {
        }

        void Update(float deltaTime) override
        {
            if (m_TotalSpawned >= m_MaxTotalEnemies) return;

            m_SpawnedEnemies.erase(std::remove_if(m_SpawnedEnemies.begin(), m_SpawnedEnemies.end(), [](GameObject* e) { return e == nullptr || e->IsMarkedForDestroy(); }), m_SpawnedEnemies.end());

            if (m_SpawnedEnemies.size() < m_MaxConcurrent)
            {
                m_SpawnTimer -= deltaTime;
                if (m_SpawnTimer <= 0.0f)
                {
                    m_SpawnTimer = 6.0f; // 6s between spawns
                    m_TotalSpawned++;

                    auto enemy = std::make_unique<GameObject>();
                    enemy->AddComponent<RenderComponent>("PNG/Enemy/VNOB1.png");
                    enemy->AddComponent<EnemyComponent>(m_p1);

                    auto myPos = GetOwner()->GetTransform().GetPosition();
                    enemy->SetLocalPosition(myPos.x, myPos.y);
                    enemy->SetZIndex(4);

                    m_SpawnedEnemies.push_back(enemy.get());

                    auto allEnemies = m_p1->GetEnemies();
                    allEnemies.push_back(enemy.get());
                    m_p1->SetEnemies(allEnemies);

                    SceneManager::GetInstance().GetActiveScene()->Add(std::move(enemy));
                }
            }
        }

    private:
        DiggerComponent* m_p1;
        int m_MaxTotalEnemies;
        int m_MaxConcurrent;
        int m_TotalSpawned{ 0 };
        float m_SpawnTimer{ 0.0f }; // spawn 1 enemy immediately
        std::vector<GameObject*> m_SpawnedEnemies;
    };
}
#endif