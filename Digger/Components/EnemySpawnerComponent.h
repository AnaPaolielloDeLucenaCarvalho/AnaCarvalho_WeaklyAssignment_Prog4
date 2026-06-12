#ifndef ENEMY_SPAWNER_COMPONENT_H
#define ENEMY_SPAWNER_COMPONENT_H

#include "Component.h"
#include "Subject.h"
#include "GameObject.h"

#include <vector>

// DESIGN PATTERN - Observer Pattern
// The spawner contains a Subject so it can broadcast an "EnemyThresholdReached" event into the wild. When the spawner reaches 75% capacity, it shouts the event, and the completely decoupled LevelManager hears it and spawns the Cherry power-up. No tight coupling required!

namespace dae
{
    class DiggerComponent;
    class Observer;

    class EnemySpawnerComponent final : public Component
    {
    public:
        EnemySpawnerComponent(GameObject* owner, DiggerComponent* p1, DiggerComponent* p2, int maxTotal, int maxConcurrent);

        void Render() const override {}
        void Update(float deltaTime) override;

        void AddObserver(Observer* obs);

    private:
        DiggerComponent* m_p1;
        DiggerComponent* m_p2;
        int m_maxTotalEnemies;
        int m_maxConcurrent;
        int m_totalSpawned{ 0 };
        float m_spawnTimer{ 0.0f }; // spawn 1 enemy immediately
        bool m_thresholdNotified{ false }; // fire only once per level
        std::vector<GameObject*> m_spawnedEnemies;
        Subject m_subject;
    };
}

#endif // ENEMY_SPAWNER_COMPONENT_H