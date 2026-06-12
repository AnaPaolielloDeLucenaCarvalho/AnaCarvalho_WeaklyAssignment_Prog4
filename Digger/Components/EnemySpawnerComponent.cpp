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
    EnemySpawnerComponent::EnemySpawnerComponent(GameObject* owner, DiggerComponent* p1, DiggerComponent* p2, int maxTotal, int maxConcurrent)
        : Component(owner)
        , m_p1(p1)
        , m_p2(p2)
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
        // Stop spawning entirely if we have exhausted the level's total enemy quota
        if (m_TotalSpawned >= m_MaxTotalEnemies) return;

        // Clean up the tracking array. Erase-remove idiom safely removes dangling pointers to dead enemies.
        m_SpawnedEnemies.erase
        (
            std::remove_if(m_SpawnedEnemies.begin(), m_SpawnedEnemies.end(), [](GameObject* e) { return e == nullptr || e->IsMarkedForDestroy(); }), m_SpawnedEnemies.end()
        );

        // Reset the timer instantly if the board is completely empty so the player isn't waiting around
        if (m_SpawnedEnemies.empty() && m_TotalSpawned < m_MaxTotalEnemies)
        {
            m_SpawnTimer = 0.0f;
        }

        // Only spawn a new enemy if we haven't hit the screen-concurrency cap
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
                enemy->AddComponent<EnemyComponent>(m_p1, m_p2);

                const auto myPos = GetOwner()->GetTransform().GetPosition();
                enemy->SetLocalPosition(myPos.x, myPos.y);
                enemy->SetZIndex(4);

                m_SpawnedEnemies.push_back(enemy.get());

                // Register with the player so fireballs can successfully calculate collision targets
                m_p1->AddEnemy(enemy.get());
                if (m_p2) m_p2->AddEnemy(enemy.get());

                SceneManager::GetInstance().GetActiveScene()->Add(std::move(enemy));

                // DESIGN PATTERN - Observer (Broadcast)
                // Fire the trigger for the bonus Cherry when 75% of the total enemies have spawned
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
