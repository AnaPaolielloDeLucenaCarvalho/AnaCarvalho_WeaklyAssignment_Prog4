#include "EnemySpawnerComponent.h"
#include "LevelManager.h"

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
        , m_maxTotalEnemies(maxTotal)
        , m_maxConcurrent(maxConcurrent)
    {
    }

    void EnemySpawnerComponent::AddObserver(Observer* obs)
    {
        m_subject.AddObserver(obs);
    }

    void EnemySpawnerComponent::Update(float deltaTime)
    {
        if (m_p1 && m_p1->IsDead()) return;
        if (LevelManager::GetInstance().GetGameMode() != GameMode::SinglePlayer && m_p2 && m_p2->IsDead()) return;

        // Stop spawning entirely if we have exhausted the level's total enemy quota
        if (m_totalSpawned >= m_maxTotalEnemies) return;

        // Clean up the tracking array. Erase-remove idiom safely removes dangling pointers to dead enemies.
        m_spawnedEnemies.erase
        (
            std::remove_if(m_spawnedEnemies.begin(), m_spawnedEnemies.end(), [](GameObject* e) { return e == nullptr || e->IsMarkedForDestroy(); }), m_spawnedEnemies.end()
        );

        // Reset the timer instantly if the board is completely empty so the player isn't waiting around
        if (m_spawnedEnemies.empty() && m_totalSpawned < m_maxTotalEnemies)
        {
            m_spawnTimer = 0.0f;
        }

        // Only spawn a new enemy if we haven't hit the screen-concurrency cap
        if (static_cast<int>(m_spawnedEnemies.size()) < m_maxConcurrent)
        {
            m_spawnTimer -= deltaTime;
            if (m_spawnTimer <= 0.0f)
            {
                m_spawnTimer = 6.0f; // 6 seconds between spawns
                m_totalSpawned++;

                // Spawn exactly at the owner (the 'E' tile) position
                auto enemy = std::make_unique<GameObject>();
                enemy->AddComponent<RenderComponent>("PNG/Enemy/VNOB1.png");
                enemy->AddComponent<EnemyComponent>(m_p1, m_p2);

                const auto myPos = GetOwner()->GetTransform().GetPosition();
                enemy->SetLocalPosition(myPos.x, myPos.y);
                enemy->SetZIndex(4);

                m_spawnedEnemies.push_back(enemy.get());

                // Register with the player so fireballs can successfully calculate collision targets
                m_p1->AddEnemy(enemy.get());
                if (m_p2) m_p2->AddEnemy(enemy.get());

                SceneManager::GetInstance().GetActiveScene()->Add(std::move(enemy));

                // DESIGN PATTERN - Observer (Broadcast)
                // Fire the trigger for the bonus Cherry when 75% of the total enemies have spawned
                if (!m_thresholdNotified)
                {
                    const int threshold = (m_maxTotalEnemies * 3) / 4; // integer 75%
                    if (m_totalSpawned >= threshold)
                    {
                        m_subject.Notify(make_sdbm_hash("EnemyThresholdReached"), m_totalSpawned);
                        m_thresholdNotified = true;
                    }
                }
            }
        }
    }
}
