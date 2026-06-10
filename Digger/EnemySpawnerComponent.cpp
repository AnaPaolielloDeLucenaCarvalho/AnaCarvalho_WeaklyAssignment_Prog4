#include "EnemySpawnerComponent.h"

#include "GameObject.h"
#include "SceneManager.h"
#include "Scene.h"
#include "DiggerComponent.h"
#include "EnemyComponent.h"
#include "RenderComponent.h"
#include "Observer.h"

#include <algorithm>

namespace dae
{
    EnemySpawnerComponent::EnemySpawnerComponent(GameObject* owner, DiggerComponent* p1, int maxTotal, int maxConcurrent)
        : Component(owner)
        , m_p1(p1)
        , m_MaxTotalEnemies(maxTotal)
        , m_MaxConcurrent(maxConcurrent)
    {
    }

    void EnemySpawnerComponent::AddObserver(Observer* obs)
    {
        m_Subject.AddObserver(obs);
    }

    void EnemySpawnerComponent::Update(float deltaTime)
    {
        if (m_TotalSpawned >= m_MaxTotalEnemies) return;

        // Remove dead / destroyed enemies from the tracking list
        m_SpawnedEnemies.erase
        (
            std::remove_if(m_SpawnedEnemies.begin(), m_SpawnedEnemies.end(), [](GameObject* e) { return e == nullptr || e->IsMarkedForDestroy(); }), m_SpawnedEnemies.end()
        );

        if (m_SpawnedEnemies.empty() && m_TotalSpawned < m_MaxTotalEnemies)
        {
            m_SpawnTimer = 0.0f;
        }

        if (static_cast<int>(m_SpawnedEnemies.size()) < m_MaxConcurrent)
        {
            m_SpawnTimer -= deltaTime;
            if (m_SpawnTimer <= 0.0f)
            {
                m_SpawnTimer = 6.0f; // 6 seconds between spawns
                m_TotalSpawned++;

                // Spawn exactly at the owner (the 'E' tile) position
                auto enemy = std::make_unique<GameObject>();
                enemy->AddComponent<RenderComponent>("PNG/Enemy/VNOB1.png");
                enemy->AddComponent<EnemyComponent>(m_p1);

                const auto myPos = GetOwner()->GetTransform().GetPosition();
                enemy->SetLocalPosition(myPos.x, myPos.y);
                enemy->SetZIndex(4);

                m_SpawnedEnemies.push_back(enemy.get());

                // Register with the player so fireballs can target this enemy
                auto allEnemies = m_p1->GetEnemies();
                allEnemies.push_back(enemy.get());
                m_p1->SetEnemies(allEnemies);

                SceneManager::GetInstance().GetActiveScene()->Add(std::move(enemy));

                if (!m_ThresholdNotified)
                {
                    const int threshold = (m_MaxTotalEnemies * 3) / 4; // integer 75%
                    if (m_TotalSpawned >= threshold)
                    {
                        m_Subject.Notify(make_sdbm_hash("EnemyThresholdReached"), m_TotalSpawned);
                        m_ThresholdNotified = true;
                    }
                }
            }
        }
    }
}
