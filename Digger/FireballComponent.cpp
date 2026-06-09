#include "FireballComponent.h"

#include "DiggerComponent.h"
#include "RenderComponent.h"
#include "LevelManager.h"
#include "GameObject.h"

#include <glm/glm.hpp>
#include <string>

namespace dae
{
    FireballComponent::FireballComponent(GameObject* owner,
                                          const glm::vec2& dir,
                                          DiggerComponent* pDigger)
        : Component(owner)
        , m_Direction(dir)
        , m_pDigger(pDigger)
    {
    }

    void FireballComponent::Update(float deltaTime)
    {
        // Explosion phase — animate frames then destroy
        if (m_IsExploding)
        {
            m_ExplodeTimer += deltaTime;
            if (m_ExplodeTimer > 0.1f)
            {
                m_ExplodeTimer -= 0.1f;
                m_ExplodeFrame++;

                if (m_ExplodeFrame > 3)
                {
                    GetOwner()->MarkForDestroy();
                }
                else if (auto* render = GetOwner()->GetComponent<RenderComponent>())
                {
                    render->SetTexture("PNG/Other/VEXP" + std::to_string(m_ExplodeFrame) + ".png");
                }
            }
            return; // Freeze position during explosion
        }

        // Movement phase
        const float speed = 300.0f * deltaTime;
        const auto  pos   = GetOwner()->GetTransform().GetPosition();
        const float newX  = pos.x + (m_Direction.x * speed);
        const float newY  = pos.y + (m_Direction.y * speed);

        GetOwner()->SetLocalPosition(newX, newY);

        // Fireball animation cycle
        m_AnimTimer += deltaTime;
        if (m_AnimTimer > 0.1f)
        {
            m_AnimTimer -= 0.1f;
            m_Frame++;
            if (m_Frame > 3) m_Frame = 1;

            if (auto* render = GetOwner()->GetComponent<RenderComponent>())
            {
                render->SetTexture("PNG/Other/VFIRE" + std::to_string(m_Frame) + ".png");
            }
        }

        // Enemy hit detection
        if (m_pDigger)
        {
            for (auto* enemy : m_pDigger->GetEnemies())
            {
                if (!enemy || enemy->IsMarkedForDestroy()) continue;

                const auto ePos = enemy->GetTransform().GetPosition();
                if (glm::distance(glm::vec2(newX, newY), glm::vec2(ePos.x, ePos.y)) < 25.f)
                {
                    enemy->MarkForDestroy();
                    m_pDigger->AwardPoints(250); // 250 pts per enemy killed by fireball

                    m_IsExploding = true;
                    if (auto* render = GetOwner()->GetComponent<RenderComponent>())
                        render->SetTexture("PNG/Other/VEXP1.png");

                    return; // Only one hit registered per frame
                }
            }
        }

        // Wall / out-of-bounds detection — explode on impact
        if (LevelManager::GetInstance().IsDirtAt(newX, newY) ||
            newX < 0.f || newX > 1040.f || newY < 0.f || newY > 612.f)
        {
            m_IsExploding = true;
            if (auto* render = GetOwner()->GetComponent<RenderComponent>())
            {
                render->SetTexture("PNG/Other/VEXP1.png");
            }
        }
    }
}
