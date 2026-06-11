#include "DiggerComponent.h"
#include "DiggerState.h"
#include "ServiceLocator.h"
#include "DiggerSounds.h"

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
        if (m_FireballCooldown > 0.0f) m_FireballCooldown -= deltaTime;
        if (m_ShootAnimTimer > 0.0f) m_ShootAnimTimer -= deltaTime;

        if (m_pCurrentState)
        {
            DiggerState* newState = m_pCurrentState->Update(this, deltaTime);
            if (newState != nullptr)
            {
                ChangeState(newState);
            }
        }

        m_DesiredDirection = { 0, 0 };
    }

    void DiggerComponent::ChangeState(DiggerState* newState)
    {
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
        // If level won make digger invincible
        if (dynamic_cast<DiggerLevelCompleteState*>(m_pCurrentState.get()) != nullptr)
        {
            return;
        }

        if (m_Lives > 0)
        {
            m_Lives--;
            m_Subject.Notify(make_sdbm_hash("PlayerDied"), m_Lives);
        }

        if (m_Lives <= 0)
        {
			// TODO - actually implement game over screen
            std::cout << "GAME OVER! Going back to Main Menu...\n";
        }
    }

    void DiggerComponent::Shoot()
    {
        if (!m_IsPlayerOne && LevelManager::GetInstance().GetGameMode() == GameMode::Versus) return;

        // shoot if cooldown ready and alive
        if (m_FireballCooldown <= 0.0f && !m_IsDead)
        {
			m_FireballCooldown = 3.0f; // shooting mechanic has to wait 3s before used again
            m_ShootAnimTimer = 0.2f;

            ServiceLocator::GetSoundSystem().Play(DiggerSounds::SHOOT, 0.5f);

            auto fireball = std::make_unique<GameObject>();
            fireball->AddComponent<RenderComponent>("PNG/Other/VFIRE1.png");

			fireball->AddComponent<FireballComponent>(m_LastFacedDirection, this); // who shoots it

            auto myPos = GetOwner()->GetTransform().GetPosition();
            fireball->SetLocalPosition(myPos.x + (m_LastFacedDirection.x * 20.0f),
                myPos.y + (m_LastFacedDirection.y * 20.0f));
            fireball->SetZIndex(5);

            SceneManager::GetInstance().GetActiveScene()->Add(std::move(fireball));
        }
    }

    void DiggerComponent::AwardPoints(int points)
    {
        if (LevelManager::GetInstance().GetGameMode() == GameMode::Versus) return;

        GameMode mode = LevelManager::GetInstance().GetGameMode();

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

        if (m_pHighScoreManager)
        {
            m_pHighScoreManager->UpdateCurrentScore(m_TotalScore);
        }

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
            ServiceLocator::GetSoundSystem().Play(DiggerSounds::COMBO_8_EMES, 1.0f);
            m_ConsecutiveEmeralds = 0;
        }
    }

    void DiggerComponent::ActivateBonusMode()
    {
        ChangeState(new DiggerBonusState());
    }

    bool DiggerComponent::IsInBonusMode() const
    {
        return dynamic_cast<DiggerBonusState*>(m_pCurrentState.get()) != nullptr;
    }
}