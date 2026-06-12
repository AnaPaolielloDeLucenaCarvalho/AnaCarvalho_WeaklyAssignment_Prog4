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
        // Transition - If the dirt under the bag is removed, switch to the wobble phase.
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

        if (auto render = bag->GetOwner()->GetComponent<RenderComponent>()) 
        {
            render->SetTexture("PNG/Money/CSBAG.png");
        }
    }

    GoldBagState* GoldBagWobbleState::Update(GoldBagComponent* bag, float deltaTime)
    {
        m_WobbleTimer -= deltaTime;
        m_ToggleTimer += deltaTime;

        // Toggle the texture left and right to visually so player knows to get out of the way
        if (m_ToggleTimer > 0.15f)
        {
            m_ToggleTimer -= 0.15f;
            m_IsLeft = !m_IsLeft;

            if (auto render = bag->GetOwner()->GetComponent<RenderComponent>()) 
            {
                render->SetTexture(m_IsLeft ? "PNG/Money/CLBAG.png" : "PNG/Money/CRBAG.png");
            }
        }

        // Transition - after 2s of wobbling, gravity takes over
        if (m_WobbleTimer <= 0.0f)
        {
            return new GoldBagFallingState();
        }
        return nullptr;
    }

// falling state
    void GoldBagFallingState::OnEnter(GoldBagComponent* bag)
    {
        // Record the starting height so we know if the bag fell far enough to break open
        m_StartY = bag->GetOwner()->GetTransform().GetPosition().y;
    }

    GoldBagState* GoldBagFallingState::Update(GoldBagComponent* bag, float deltaTime)
    {
        auto pos = bag->GetOwner()->GetTransform().GetPosition();
        float speed = 150.f * deltaTime;
        float newY = pos.y + speed;

        float gridSize = LevelManager::GetInstance().GetGridSize();
        float offsetY = LevelManager::GetInstance().GetOffsetY();

        // Calculate the nearest grid intersection to snap to when landing
        float snappedY = std::round((newY - offsetY) / gridSize) * gridSize + offsetY;

        // Check if we are close enough to a grid tile to process a landing check
        if (std::abs(newY - snappedY) <= speed)
        {
            // Stop falling if we hit solid dirt or the bottom of the play area
            if (LevelManager::GetInstance().IsDirtAt(pos.x, snappedY + gridSize) || snappedY >= 576.f - gridSize)
            {
                bag->GetOwner()->SetLocalPosition(pos.x, snappedY);

                // Transition - If it fell more than 1.5 tiles, it shatters into collectable gold!
                if ((snappedY - m_StartY) > (gridSize * 1.5f))
                {
                    return new GoldBagBrokenState();
                }
                else
                {
                    // If it barely fell, it just settles safely back into an idle state.
                    return new GoldBagIdleState();
                }
            }
        }

        // Apply movement and trigger the death hitbox for anything beneath the bag
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
        // If the dirt supporting the broken gold is removed, the gold is permanently destroyed.
        if (!bag->IsDirtDirectlyUnderneath() && !bag->HitBottom())
        {
            bag->GetOwner()->MarkForDestroy();
            return nullptr;
        }

        m_DespawnTimer -= deltaTime;
        m_AnimTimer += deltaTime;

        // Cycle through the glittering gold animation frames (small -> big -> settled)
        if (m_AnimTimer > 0.2f && m_GoldFrame == 1)
        {
            m_GoldFrame = 2;
            if (auto render = bag->GetOwner()->GetComponent<RenderComponent>()) 
            {
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

        // Cleanup - Gold vanishes if not collected fast enough.
        if (m_DespawnTimer <= 0.0f)
        {
            bag->GetOwner()->MarkForDestroy();
        }
        return nullptr;
    }
}