#include "GoldBagComponent.h"
#include "GoldBagState.h"
#include "LevelManager.h"
#include "DiggerComponent.h"
#include "DiggerState.h"
#include "ServiceLocator.h"
#include "AudioDefinitions.h"
#include <cmath>

namespace dae
{
    GoldBagComponent::GoldBagComponent(dae::GameObject* owner)
        : dae::Component(owner)
    {
        // Bags always spawn completely idle, waiting for the dirt underneath to be removed
        m_pCurrentState = std::make_unique<GoldBagIdleState>();
        m_pCurrentState->OnEnter(this);
    }

    GoldBagComponent::~GoldBagComponent()
    {
        if (m_pCurrentState)
        {
            m_pCurrentState->OnExit(this);
        }
    }

    void GoldBagComponent::Update(float deltaTime)
    {
        // Helper lambda - Determines if a given player is currently dead or transitioning levels
        auto isPaused = [](GameObject* playerObj) 
        {
            if (!playerObj) return false;
            if (auto digger = playerObj->GetComponent<DiggerComponent>())
            {
                return digger->IsDead() || digger->IsLevelComplete();
            }
            return true; // if no digger comp, assume paused/dead
        };

        bool p1Paused = isPaused(m_pPlayer1);
        bool p2Paused = m_pPlayer2 ? isPaused(m_pPlayer2) : true;

        // Freeze physics if the game is essentially "paused" due to player deaths
        if (LevelManager::GetInstance().GetGameMode() == GameMode::SinglePlayer)
        {
            if (p1Paused) return;
        }
        else
        {
            // In Co-Op or Versus, bags only pause if BOTH players are paused/dead
            if (p1Paused && p2Paused) return;
        }

        // Delegate execution down to the active State (Idle, Wobble, Falling, Broken)
        if (m_pCurrentState)
        {
            GoldBagState* newState = m_pCurrentState->Update(this, deltaTime);
            if (newState != nullptr)
            {
                ChangeState(newState);
            }
        }
    }

    void GoldBagComponent::ChangeState(GoldBagState* newState)
    {
        // Cleanly exit the current state before entering the next one to avoid logic leaks
        if (m_pCurrentState)
        {
            m_pCurrentState->OnExit(this);
        }

        m_pCurrentState.reset(newState);

        if (m_pCurrentState)
        {
            m_pCurrentState->OnEnter(this);
        }
    }

    bool GoldBagComponent::IsDirtDirectlyUnderneath() const
    {
        // Check exactly 1 grid tile (40 pixels) directly below the bag's current position
        auto myPos = GetOwner()->GetTransform().GetPosition();
        return LevelManager::GetInstance().IsDirtAt(myPos.x, myPos.y + 40.0f);
    }

    bool GoldBagComponent::HitBottom() const
    {
        // Hardcoded safety check to prevent bags from falling out of the map bounds
        auto myPos = GetOwner()->GetTransform().GetPosition();
        return (myPos.y >= 540.f);
    }

    void GoldBagComponent::SmashEntitiesBelow()
    {
        auto myPos = GetOwner()->GetTransform().GetPosition();

        // Helper lambda: Checks if a player is caught underneath the falling bag and crushes them
        auto checkAndSmash = [&](GameObject* player)
            {
                if (player && !player->IsMarkedForDestroy())
                {
                    auto pPos = player->GetTransform().GetPosition();

                    // Strict vertical bounding box check (must be aligned X, and below on Y)
                    if (std::abs(pPos.x - myPos.x) < 20.f && (pPos.y - myPos.y) > 0 && (pPos.y - myPos.y) < 30.f)
                    {
                        if (auto diggerComp = player->GetComponent<DiggerComponent>())
                        {
                            if (!diggerComp->IsDead() && diggerComp->GetLives() > 0)
                            {
                                // Force the player directly into the Dead state, ignoring normal health rules!
                                diggerComp->ChangeState(new DiggerDeadState());
                            }
                        }
                    }
                }
            };

        checkAndSmash(m_pPlayer1);

        // Check the second player (Co-Op or Versus)
        if (m_pPlayer2 && !m_pPlayer2->IsMarkedForDestroy()) 
        {
            auto p2Pos = m_pPlayer2->GetTransform().GetPosition();
            if (glm::distance(glm::vec2(myPos.x, myPos.y), glm::vec2(p2Pos.x, p2Pos.y)) < 24.f) 
            {
                if (auto p2Digger = m_pPlayer2->GetComponent<DiggerComponent>()) 
                {
                    p2Digger->ChangeState(new dae::DiggerDeadState());
                }
            }
        }

        // Crush Enemies
        if (m_pPlayer1)
        {
            if (auto digger = m_pPlayer1->GetComponent<DiggerComponent>())
            {
                for (auto& enemy : digger->GetEnemies())
                {
                    if (!enemy || enemy->IsMarkedForDestroy()) continue;
                    auto ePos = enemy->GetTransform().GetPosition();

                    // If an enemy is caught underneath, destroy it and award the player massive points
                    if (std::abs(ePos.x - myPos.x) < 20.f && (ePos.y - myPos.y) > 0 && (ePos.y - myPos.y) < 30.f)
                    {
                        enemy->MarkForDestroy(); // crush the enemy
                        digger->AwardPoints(250);
                        ServiceLocator::GetSoundSystem().Play(AudioDefinitions::KILL_ENEMY, 0.5f);
                    }
                }
            }
        }
    }
}