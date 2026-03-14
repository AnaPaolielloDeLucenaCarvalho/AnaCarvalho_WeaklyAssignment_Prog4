#ifndef DIGGER_COMPONENT_H
#define DIGGER_COMPONENT_H

#include "Component.h"
#include "Subject.h"
#include "GameObject.h"
#include <vector>
#include <glm/glm.hpp>

namespace dae
{
    class DiggerComponent : public Component, public Subject
    {
    public:
        DiggerComponent(GameObject* owner) : Component(owner) {}
        virtual ~DiggerComponent() = default;

        void SetOtherPlayer(GameObject* pOther) { m_pOtherPlayer = pOther; }
        void SetDiamonds(const std::vector<GameObject*>& diamonds) { m_pDiamonds = diamonds; }

        void Update(float deltaTime) override
        {
            auto myPos = GetOwner()->GetTransform().GetPosition();

            if (m_Invincible)
            {
                m_InvincibleTimer -= deltaTime;
                if (m_InvincibleTimer <= 0)
                {
                    m_Invincible = false;
                }
            }

            // COLLISION WITH DIAMONDS (Scoring)
            for (auto& diamond : m_pDiamonds)
            {
                if (!diamond || diamond->IsMarkedForDestroy())
                {
                    continue;
                }

                auto diamondPos = diamond->GetTransform().GetPosition();
                float dist = glm::distance(glm::vec2(myPos.x, myPos.y), glm::vec2(diamondPos.x, diamondPos.y));

                if (dist < 20.f)
                {
                    diamond->MarkForDestroy();
                    GainScore(100);
                }
            }

            // COLLISION WITH OTHER PLAYER (Losing Lives)
            if (m_pOtherPlayer && !m_Invincible)
            {
                auto otherPos = m_pOtherPlayer->GetTransform().GetPosition();
                float dist = glm::distance(glm::vec2(myPos.x, myPos.y), glm::vec2(otherPos.x, otherPos.y));

                if (dist < 25.f)
                {
                    Die();
                    StartInvincibilityFrames();
                }
            }
        }

        void Die()
        {
            if (m_Lives > 0)
            {
                m_Lives--;
                Notify(Event::PlayerDied, m_Lives); // Ex1
            }
        }

        void GainScore(int amount)
        {
            m_Score += amount;
            Notify(Event::DiamondPickedUp, m_Score); // Ex2
        }

        void StartInvincibilityFrames()
        {
            m_Invincible = true;
            m_InvincibleTimer = 2.0f;
        }

    private:
        int m_Score = 0;
        int m_Lives = 3;

        GameObject* m_pOtherPlayer{ nullptr };
        std::vector<GameObject*> m_pDiamonds{};

        bool m_Invincible{ false };
        float m_InvincibleTimer{ 0.0f };
    };
}
#endif