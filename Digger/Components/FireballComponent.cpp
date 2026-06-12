#include "FireballComponent.h"
#include "DiggerComponent.h"
#include "DiggerState.h"
#include "RenderComponent.h"
#include "LevelManager.h"
#include "GameObject.h"
#include "ServiceLocator.h"
#include "AudioDefinitions.h"

#include <glm/glm.hpp>
#include <string>

namespace dae
{
    FireballComponent::FireballComponent(GameObject* owner, const glm::vec2& dir, DiggerComponent* pDigger)
        : Component(owner)
        , m_Direction(dir)
        , m_pDigger(pDigger)
    {
    }

    void FireballComponent::Update(float deltaTime)
    {
        // Guard Clause (Early Return) - If the fireball is already exploding, process the animation and exit immediately.
        if (m_IsExploding)
        {
            m_ExplodeTimer += deltaTime;
            if (m_ExplodeTimer > 0.1f)
            {
                m_ExplodeTimer -= 0.1f;
                m_ExplodeFrame++;

                if (m_ExplodeFrame > 3)
                {
                    // RAII / Cleanup - Mark the object for destruction so the engine safely deletes it at the end of the frame.
                    GetOwner()->MarkForDestroy();
                }
                else if (auto* render = GetOwner()->GetComponent<RenderComponent>())
                {
                    render->SetTexture("PNG/Other/VEXP" + std::to_string(m_ExplodeFrame) + ".png");
                }
            }
            return; // Freeze position during explosion
        }

        // Variable Time Step (Delta Time) - Multiply speed by deltaTime so the fireball travels  at the exact same speed regardless of the player's frame rate.
        const float speed = 300.0f * deltaTime;
        const auto pos = GetOwner()->GetTransform().GetPosition();
        const float newX = pos.x + (m_Direction.x * speed);
        const float newY = pos.y + (m_Direction.y * speed);

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

        // DESIGN DECISION: Dynamic Collision Checks
        // We query the Digger's enemy list to find valid targets.
        if (m_pDigger)
        {
            for (auto* enemy : m_pDigger->GetEnemies())
            {
                if (!enemy || enemy->IsMarkedForDestroy()) continue;

                const auto ePos = enemy->GetTransform().GetPosition();
                if (glm::distance(glm::vec2(newX, newY), glm::vec2(ePos.x, ePos.y)) < 25.f)
                {
                    enemy->MarkForDestroy();
                    m_pDigger->AwardPoints(250);

                    // DESIGN PATTERN: Service Locator
                    // Safely triggers the audio system globally without needing a hardcoded Audio pointer in this class.
                    ServiceLocator::GetSoundSystem().Play(AudioDefinitions::KILL_ENEMY, 0.5f);

                    m_IsExploding = true;
                    if (auto* render = GetOwner()->GetComponent<RenderComponent>()) render->SetTexture("PNG/Other/VEXP1.png");

                    return; // Early return guarantees only ONE enemy takes damage per frame
                }
            }
        }

        // Versus Mode Specific Collision
        if (LevelManager::GetInstance().GetGameMode() == GameMode::Versus) 
        {
            auto p2 = m_pDigger->GetOtherPlayer();

            if (p2 && !p2->IsMarkedForDestroy()) 
            {
                auto p2Pos = p2->GetTransform().GetPosition();
                if (glm::distance(glm::vec2(newX, newY), glm::vec2(p2Pos.x, p2Pos.y)) < 20.f) 
                {
                    // DESIGN PATTERN - State Pattern
                    // Instead of setting `isDead = true`, we transition Player 2 cleanly into the Dead State.
                    p2->GetComponent<DiggerComponent>()->ChangeState(new dae::DiggerDeadState());
                    GetOwner()->MarkForDestroy();
                    return;
                }
            }
        }

        // Wall / out-of-bounds detection
        // DESIGN PATTERN - Singleton access for LevelManager
        if (LevelManager::GetInstance().IsDirtAt(newX, newY) || newX < 0.f || newX > 1040.f || newY < 0.f || newY > 612.f)
        {
            m_IsExploding = true;
            if (auto* render = GetOwner()->GetComponent<RenderComponent>())
            {
                render->SetTexture("PNG/Other/VEXP1.png");
            }
        }
    }
}
