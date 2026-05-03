#include "GoldBagState.h"
#include "GoldBagComponent.h"
#include "RenderComponent.h"
#include "LevelManager.h"
#include <iostream>

namespace dae
{
	// idle state
    GoldBagState* GoldBagIdleState::Update(GoldBagComponent* bag, float /*deltaTime*/)
    {
        if (!bag->IsDirtDirectlyUnderneath())
        {
            return new GoldBagWobbleState();
        }
        return nullptr;
    }

	// wobble state
    void GoldBagWobbleState::OnEnter(GoldBagComponent* bag)
    {
        m_WobbleTimer = 2.0f;

        if (auto render = bag->GetOwner()->GetComponent<RenderComponent>()) {
            render->SetTexture("PNG/Money/CSBAG.png");
        }
    }

    GoldBagState* GoldBagWobbleState::Update(GoldBagComponent* bag, float deltaTime)
    {
        m_WobbleTimer -= deltaTime;
        m_ToggleTimer += deltaTime;

        if (m_ToggleTimer > 0.15f)
        {
            m_ToggleTimer -= 0.15f;
            m_IsLeft = !m_IsLeft;

            if (auto render = bag->GetOwner()->GetComponent<RenderComponent>()) {
                render->SetTexture(m_IsLeft ? "PNG/Money/CLBAG.png" : "PNG/Money/CRBAG.png");
            }
        }

        if (m_WobbleTimer <= 0.0f)
        {
            return new GoldBagFallingState();
        }
        return nullptr;
    }

	// falling state
    void GoldBagFallingState::OnEnter(GoldBagComponent* bag)
    {
        m_StartY = bag->GetOwner()->GetTransform().GetPosition().y;
    }

    GoldBagState* GoldBagFallingState::Update(GoldBagComponent* bag, float deltaTime)
    {
        auto pos = bag->GetOwner()->GetTransform().GetPosition();
        float speed = 150.f * deltaTime;
        float newY = pos.y + speed;

        float gridSize = LevelManager::GetInstance().GetGridSize();
        float offsetY = LevelManager::GetInstance().GetOffsetY();

        float snappedY = std::round((newY - offsetY) / gridSize) * gridSize + offsetY;

        if (std::abs(newY - snappedY) <= speed)
        {
            if (LevelManager::GetInstance().IsDirtAt(pos.x, snappedY + gridSize) || snappedY >= 576.f - gridSize)
            {
                bag->GetOwner()->SetLocalPosition(pos.x, snappedY);

                if ((snappedY - m_StartY) > (gridSize * 1.5f))
                {
                    return new GoldBagBrokenState();
                }
                else
                {
                    return new GoldBagIdleState();
                }
            }
        }

        bag->GetOwner()->SetLocalPosition(pos.x, newY);
        bag->SmashEntitiesBelow();

        return nullptr;
    }

	// broken state
    void GoldBagBrokenState::OnEnter(GoldBagComponent* bag)
    {
        bag->SetBroken(true);

        if (auto render = bag->GetOwner()->GetComponent<RenderComponent>())
        {
            render->SetTexture("PNG/Money/VGOLD1.png");
        }
    }

    GoldBagState* GoldBagBrokenState::Update(GoldBagComponent* bag, float deltaTime)
    {
        if (!bag->IsDirtDirectlyUnderneath())
        {
            bag->GetOwner()->MarkForDestroy();
            return nullptr;
        }

        m_DespawnTimer -= deltaTime;
        m_AnimTimer += deltaTime;

        // small -> big -> settled
        if (m_AnimTimer > 0.2f && m_GoldFrame == 1)
        {
            m_GoldFrame = 2;
            if (auto render = bag->GetOwner()->GetComponent<RenderComponent>()) {
                render->SetTexture("PNG/Money/VGOLD2.png");
            }
        }
        else if (m_AnimTimer > 0.4f && m_GoldFrame == 2)
        {
            m_GoldFrame = 3;
            if (auto render = bag->GetOwner()->GetComponent<RenderComponent>())
            {
                render->SetTexture("PNG/Money/VGOLD3.png");
            }
        }

        if (m_DespawnTimer <= 0.0f)
        {
            bag->GetOwner()->MarkForDestroy();
        }
        return nullptr;
    }
}