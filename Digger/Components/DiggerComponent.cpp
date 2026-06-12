#include "DiggerComponent.h"
#include "DiggerState.h"
#include "ServiceLocator.h"
#include "AudioDefinitions.h"

#include "FireballComponent.h"
#include "SceneManager.h"
#include "Scene.h"
#include "HighScoreManager.h"

#include <iostream>
#include <RenderComponent.h>
#include "LevelManager.h"

namespace dae
{
    DiggerComponent::DiggerComponent(GameObject* owner) 
        : Component(owner)
    {
        m_FireballCooldown = 0.3f;

        // Initialize Digger into the Normal gameplay state immediately upon creation
        m_pCurrentState = std::make_unique<DiggerNormalState>();
        m_pCurrentState->OnEnter(this);
    }

    DiggerComponent::~DiggerComponent()
    {
        if (m_pCurrentState)
        {
            m_pCurrentState->OnExit(this);
        }
    }

    void DiggerComponent::Update(float deltaTime)
    {
        // Tick down action timers
        if (m_FireballCooldown > 0.0f) m_FireballCooldown -= deltaTime;
        if (m_ShootAnimTimer > 0.0f) m_ShootAnimTimer -= deltaTime;

        // DESIGN PATTERN - State Pattern (Delegation)
        // Digger handles NO physics or collision logic here. It blindly passes the Update call to its current state object, keeping this component incredibly clean
        if (m_pCurrentState)
        {
            DiggerState* newState = m_pCurrentState->Update(this, deltaTime);
            // If the state returned a new state (EX - Normal returned Dead), execute the swap
            if (newState != nullptr)
            {
                ChangeState(newState);
            }
        }

        // Reset the desired direction input every frame so movement stops if a key is released
        m_DesiredDirection = { 0, 0 };
    }

    void DiggerComponent::ChangeState(DiggerState* newState)
    {
        // Properly exit the old state to clean up effects/music before applying the new one
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

    void DiggerComponent::SetIsPlayerOne(bool isP1)
    {
        m_IsPlayerOne = isP1;
    }

    bool DiggerComponent::IsPlayerOne() const
    {
        return m_IsPlayerOne;
    }

    void DiggerComponent::Die()
    {
        // If level won make digger invincible by aborting the death call
        if (dynamic_cast<DiggerLevelCompleteState*>(m_pCurrentState.get()) != nullptr)
        {
            return;
        }

        if (m_Lives > 0)
        {
            m_Lives--;

            // Broadcast the death event so the UI updates the life sprites instantly
            m_Subject.Notify(make_sdbm_hash("PlayerDied"), m_Lives);
        }

        if (m_Lives <= 0)
        {
            std::cout << "GAME OVER! Going back to Main Menu...\n";
        }
    }

    void DiggerComponent::Shoot()
    {
        // Player 2 acts as the Enemy in Versus Mode and is not allowed to shoot fireballs
        if (!m_IsPlayerOne && LevelManager::GetInstance().GetGameMode() == GameMode::Versus) return;

        // shoot if cooldown ready and alive
        if (m_FireballCooldown <= 0.0f && !m_IsDead)
        {
            m_FireballCooldown = 3.0f; // shooting mechanic has to wait 3s before used again
            m_ShootAnimTimer = 0.2f;

            ServiceLocator::GetSoundSystem().Play(AudioDefinitions::SHOOT, 0.5f);

            // Construct and spawn the Fireball entity dynamically
            auto fireball = std::make_unique<GameObject>();
            fireball->AddComponent<RenderComponent>("PNG/Other/VFIRE1.png");
            fireball->AddComponent<FireballComponent>(m_LastFacedDirection, this); // Track who shoots it

            auto myPos = GetOwner()->GetTransform().GetPosition();
            fireball->SetLocalPosition(myPos.x + (m_LastFacedDirection.x * 20.0f), myPos.y + (m_LastFacedDirection.y * 20.0f));
            fireball->SetZIndex(5);

            SceneManager::GetInstance().GetActiveScene()->Add(std::move(fireball));
        }
    }

    void DiggerComponent::AwardPoints(int points)
    {
        // Enemy player in Versus Mode does not accumulate points
        if (LevelManager::GetInstance().GetGameMode() == GameMode::Versus) return;

        GameMode mode = LevelManager::GetInstance().GetGameMode();

        // In Co-Op mode, Digger points are pooled globally, so we forcefully sync the score numbers  and force both UI Subjects to notify their respective visual scoreboards.
        if (mode == GameMode::CoOp && m_pOtherPlayer)
        {
            m_TotalScore += points;
            m_Subject.Notify(dae::make_sdbm_hash("DiamondPickedUp"), points);

            if (auto otherDigger = m_pOtherPlayer->GetComponent<DiggerComponent>())
            {
                otherDigger->m_TotalScore = m_TotalScore;
                otherDigger->GetSubject().Notify(dae::make_sdbm_hash("DiamondPickedUp"), points);
            }
        }
        else
        {
            m_TotalScore += points;
            m_Subject.Notify(dae::make_sdbm_hash("DiamondPickedUp"), points);
        }

        // Push the ongoing score into the central memory cache for Game Over processing
        if (m_pHighScoreManager)
        {
            m_pHighScoreManager->UpdateCurrentScore(m_TotalScore);
        }

        // Arcade 1UP mechanic: Award a single extra life for surpassing 20,000 points
        if (m_TotalScore >= 20000 && !m_HasGottenExtraLife)
        {
            m_Lives++;
            m_HasGottenExtraLife = true;

            m_Subject.Notify(dae::make_sdbm_hash("PlayerDied"), m_Lives);

            // play bonus sound
            ServiceLocator::GetSoundSystem().Play(1, 1.0f);
        }
    }

    void DiggerComponent::AddEmeraldToCombo()
    {
        m_ConsecutiveEmeralds++;
        if (m_ConsecutiveEmeralds >= 8)
        {
            // award 250 points and reset combo
            AwardPoints(250);
            ServiceLocator::GetSoundSystem().Play(AudioDefinitions::COMBO_8_EMES, 1.0f);
            m_ConsecutiveEmeralds = 0;
        }
    }

    void DiggerComponent::ActivateBonusMode()
    {
        // Prevent infinite recursion and double-activations resetting the timer
        if (IsInBonusMode()) return;

        ChangeState(new DiggerBonusState());

        // Shared Co-Op Power-up Logic: If Player 1 eats a cherry, Player 2 gets the buff too!
        if (LevelManager::GetInstance().GetGameMode() == GameMode::CoOp && m_pOtherPlayer)
        {
            if (auto otherDigger = m_pOtherPlayer->GetComponent<DiggerComponent>())
            {
                // Only force them into bonus mode if they aren't already in it!
                if (!otherDigger->IsInBonusMode())
                {
                    otherDigger->ChangeState(new DiggerBonusState());
                }
            }
        }
    }

    bool DiggerComponent::IsInBonusMode() const
    {
        // Safely evaluate the active state by attempting a dynamic cast on the base pointer
        return dynamic_cast<DiggerBonusState*>(m_pCurrentState.get()) != nullptr;
    }
}