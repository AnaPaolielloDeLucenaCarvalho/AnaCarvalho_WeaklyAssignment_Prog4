#ifndef DIGGER_COMPONENT_H
#define DIGGER_COMPONENT_H
#include "Component.h"
#include "Subject.h"
#include "GameObject.h"
#include <vector>
#include <glm/vec2.hpp>

namespace dae
{
    class DiggerState;

    class DiggerComponent : public Component
    {
    public:
        DiggerComponent(GameObject* owner);
        ~DiggerComponent() override;

        void Update(float deltaTime) override;
        void Die();

        void AddObserver(Observer* obs) { m_Subject.AddObserver(obs); }
        void SetOtherPlayer(GameObject* pOther) { m_pOtherPlayer = pOther; }
        void SetDiamonds(const std::vector<GameObject*>& diamonds) { m_pDiamonds = diamonds; }
        void SetGoldBags(const std::vector<GameObject*>& bags) { m_pGoldBags = bags; }

        GameObject* GetOtherPlayer() const { return m_pOtherPlayer; }
        const std::vector<GameObject*>& GetDiamonds() const { return m_pDiamonds; }
        const std::vector<GameObject*>& GetGoldBags() const { return m_pGoldBags; }
        Subject& GetSubject() { return m_Subject; }

        int GetLives() const { return m_Lives; }

        void ChangeState(DiggerState* newState);

        void SetDesiredDirection(const glm::vec2& dir) { m_DesiredDirection = dir; }
        glm::vec2 GetDesiredDirection() const { return m_DesiredDirection; }

        void SetCurrentDirection(const glm::vec2& dir) { m_CurrentDirection = dir; }
        glm::vec2 GetCurrentDirection() const { return m_CurrentDirection; }

        void SetSpawnPos(const glm::vec2& pos) { m_SpawnPos = pos; }
        glm::vec2 GetSpawnPos() const { return m_SpawnPos; }

        bool IsDead() const { return m_IsDead; }
        void SetDead(bool dead) { m_IsDead = dead; }

    private:
        Subject m_Subject;
        int m_Lives = 3;

        GameObject* m_pOtherPlayer{ nullptr };
        std::vector<GameObject*> m_pDiamonds{};
        std::vector<GameObject*> m_pGoldBags{};

        DiggerState* m_pCurrentState{ nullptr };

        glm::vec2 m_DesiredDirection{ 0, 0 };
        glm::vec2 m_CurrentDirection{ 0, 0 };

        glm::vec2 m_SpawnPos{ 0, 0 };
        bool m_IsDead{ false };
    };
}
#endif