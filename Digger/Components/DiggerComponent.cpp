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
        m_fireballCooldown = 0.3f;

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
        if (m_fireballCooldown > 0.0f) m_fireballCooldown -= deltaTime;
        if (m_shootAnimTimer > 0.0f) m_shootAnimTimer -= deltaTime;

        // DESIGN PATTERN - State Pattern (Delegation)
        // Digger handles NO physics or collision logic here. It blindly passes the Update call to its current state object, keeping this component incredibly clean
        if (m_pCurrentState)
        {
            std::unique_ptr<dae::DiggerState> newState = m_pCurrentState->Update(this, deltaTime);
            // If the state returned a new state (EX - Normal returned Dead), execute the swap
            if (newState != nullptr)
            {
                ChangeState(std::move(newState));
            }
        }

        // Reset the desired direction input every frame so movement stops if a key is released
        m_desiredDirection = { 0, 0 };
    }

    void DiggerComponent::ChangeState(std::unique_ptr<dae::DiggerState> newState)
    {
        // Properly exit the old state to clean up effects/music before applying the new one
        if (m_pCurrentState)
        {
            m_pCurrentState->OnExit(this);
        }

        m_pCurrentState = std::move(newState);

        if (m_pCurrentState)
        {
            m_pCurrentState->OnEnter(this);
        }
    }

    void DiggerComponent::SetIsPlayerOne(bool isP1)
    {
        m_isPlayerOne = isP1;
    }

    bool DiggerComponent::IsPlayerOne() const
    {
        return m_isPlayerOne;
    }

    void DiggerComponent::Die()
    {
        // If level won make digger invincible by aborting the death call
        if (dynamic_cast<DiggerLevelCompleteState*>(m_pCurrentState.get()) != nullptr)
        {
            return;
        }

        if (m_lives > 0)
        {
            m_lives--;

            // Broadcast the death event so the UI updates the life sprites instantly
            m_subject.Notify(make_sdbm_hash("PlayerDied"), m_lives);
        }

        if (m_lives <= 0)
        {
            std::cout << "GAME OVER! Going back to Main Menu...\n";
        }
    }

    void DiggerComponent::Shoot()
    {
        // Player 2 acts as the Enemy in Versus Mode and is not allowed to shoot fireballs
        if (!m_isPlayerOne && LevelManager::GetInstance().GetGameMode() == GameMode::Versus) return;

        // shoot if cooldown ready and alive
        if (m_fireballCooldown <= 0.0f && !m_isDead)
        {
            m_fireballCooldown = 3.0f; // shooting mechanic has to wait 3s before used again
            m_shootAnimTimer = 0.2f;

            ServiceLocator::GetSoundSystem().Play(static_cast<unsigned short>(AudioDefinitions::SHOOT), 0.5f);
            ResetEmeraldCombo();

            // Construct and spawn the Fireball entity dynamically
            auto fireball = std::make_unique<GameObject>();
            fireball->AddComponent<RenderComponent>("PNG/Other/VFIRE1.png");
            fireball->AddComponent<FireballComponent>(m_lastFacedDirection, this); // Track who shoots it

            auto myPos = GetOwner()->GetTransform().GetPosition();
            fireball->SetLocalPosition(myPos.x + (m_lastFacedDirection.x * 20.0f), myPos.y + (m_lastFacedDirection.y * 20.0f));
            fireball->SetZIndex(5);

            SceneManager::GetInstance().GetActiveScene()->Add(std::move(fireball));
        }
    }

    void DiggerComponent::AwardPoints(int points, bool breaksCombo)
    {
        if (breaksCombo) ResetEmeraldCombo();

        // Enemy player in Versus Mode does not accumulate points
        if (LevelManager::GetInstance().GetGameMode() == GameMode::Versus) return;

        GameMode mode = LevelManager::GetInstance().GetGameMode();

        // In Co-Op mode, Digger points are pooled globally, so we forcefully sync the score numbers  and force both UI Subjects to notify their respective visual scoreboards.
        if (mode == GameMode::CoOp && m_pOtherPlayer)
        {
            m_totalScore += points;
            m_subject.Notify(dae::make_sdbm_hash("DiamondPickedUp"), points);

            if (auto otherDigger = m_pOtherPlayer->GetComponent<DiggerComponent>())
            {
                otherDigger->m_totalScore = m_totalScore;
                otherDigger->GetSubject().Notify(dae::make_sdbm_hash("DiamondPickedUp"), points);
            }
        }
        else
        {
            m_totalScore += points;
            m_subject.Notify(dae::make_sdbm_hash("DiamondPickedUp"), points);
        }

        // Push the ongoing score into the central memory cache for Game Over processing
        if (m_pHighScoreManager)
        {
            m_pHighScoreManager->UpdateCurrentScore(m_totalScore);
        }

        // Arcade 1UP mechanic: Award a single extra life for surpassing 20,000 points
        if (m_totalScore >= 20000 && !m_hasGottenExtraLife)
        {
            m_lives++;
            m_hasGottenExtraLife = true;

            m_subject.Notify(dae::make_sdbm_hash("PlayerDied"), m_lives);

            // play bonus sound
            ServiceLocator::GetSoundSystem().Play(static_cast<unsigned short>(AudioDefinitions::BONUS), 1.0f);
        }
    }

    void DiggerComponent::AddEmeraldToCombo()
    {
        m_consecutiveEmeralds++;
        if (m_consecutiveEmeralds >= 8)
        {
            // award 250 points and reset combo
            AwardPoints(250);
            ServiceLocator::GetSoundSystem().Play(static_cast<unsigned short>(AudioDefinitions::COMBO_8_EMES), 1.0f);
            m_consecutiveEmeralds = 0;
        }
    }

    void DiggerComponent::ActivateBonusMode()
    {
        // Prevent infinite recursion and double-activations resetting the timer
        if (IsInBonusMode()) return;

        ChangeState(std::make_unique<DiggerBonusState>());

        // Shared Co-Op Power-up Logic: If Player 1 eats a cherry, Player 2 gets the buff too!
        if (LevelManager::GetInstance().GetGameMode() == GameMode::CoOp && m_pOtherPlayer)
        {
            if (auto otherDigger = m_pOtherPlayer->GetComponent<DiggerComponent>())
            {
                // Only force them into bonus mode if they aren't already in it!
                if (!otherDigger->IsInBonusMode())
                {
                    otherDigger->ChangeState(std::make_unique<DiggerBonusState>());
                }
            }
        }
    }

    bool DiggerComponent::IsInBonusMode() const
    {
        // Safely evaluate the active state by attempting a dynamic cast on the base pointer
        return dynamic_cast<DiggerBonusState*>(m_pCurrentState.get()) != nullptr;
    }

    void DiggerComponent::AddObserver(Observer* obs) { m_subject.AddObserver(obs); }
    void DiggerComponent::SetOtherPlayer(GameObject* pOther) { m_pOtherPlayer = pOther; }
    void DiggerComponent::SetDiamonds(const std::vector<GameObject*>& diamonds) { m_pDiamonds = diamonds; }
    void DiggerComponent::SetGoldBags(const std::vector<GameObject*>& bags) { m_pGoldBags = bags; }

    const std::vector<GameObject*>& DiggerComponent::GetDiamonds() const { return m_pDiamonds; }
    const std::vector<GameObject*>& DiggerComponent::GetGoldBags() const { return m_pGoldBags; }

    GameObject* DiggerComponent::GetOtherPlayer() const { return m_pOtherPlayer; }
    Subject& DiggerComponent::GetSubject() { return m_subject; }

    int DiggerComponent::GetLives() const { return m_lives; }
    void DiggerComponent::SetLives(int lives) { m_lives = lives; m_subject.Notify(make_sdbm_hash("PlayerDied"), m_lives); }
    int DiggerComponent::GetTotalScore() const { return m_totalScore; }
    void DiggerComponent::SetTotalScore(int score) { m_totalScore = score; m_subject.Notify(make_sdbm_hash("ScoreReset"), score); }

    void DiggerComponent::SetHighScoreManager(HighScoreManager* pMgr) { m_pHighScoreManager = pMgr; }
    HighScoreManager* DiggerComponent::GetHighScoreManager() const { return m_pHighScoreManager; }

    void DiggerComponent::SetDesiredDirection(const glm::vec2& dir) { m_desiredDirection = dir; }
    glm::vec2 DiggerComponent::GetDesiredDirection() const { return m_desiredDirection; }
    void DiggerComponent::SetCurrentDirection(const glm::vec2& dir) { m_currentDirection = dir; }
    glm::vec2 DiggerComponent::GetCurrentDirection() const { return m_currentDirection; }

    void DiggerComponent::SetSpawnPos(const glm::vec2& pos) { m_spawnPos = pos; }
    glm::vec2 DiggerComponent::GetSpawnPos() const { return m_spawnPos; }

    bool DiggerComponent::IsDead() const { return m_isDead; }
    void DiggerComponent::SetDead(bool dead) { m_isDead = dead; }

    bool DiggerComponent::IsLevelComplete() const { return m_isLevelComplete; }
    void DiggerComponent::SetLevelComplete(bool complete) { m_isLevelComplete = complete; }

    void DiggerComponent::SetLastFacedDirection(const glm::vec2& dir) { m_lastFacedDirection = dir; }
    glm::vec2 DiggerComponent::GetLastFacedDirection() const { return m_lastFacedDirection; }
    float DiggerComponent::GetFireballCooldown() const { return m_fireballCooldown; }
    float DiggerComponent::GetShootAnimTimer() const { return m_shootAnimTimer; }

    void DiggerComponent::ResetEmeraldCombo() { m_consecutiveEmeralds = 0; }

    void DiggerComponent::SetEnemies(const std::vector<GameObject*>& enemies) { m_pEnemies = enemies; }
    const std::vector<GameObject*>& DiggerComponent::GetEnemies() const { return m_pEnemies; }
    void DiggerComponent::AddEnemy(GameObject* enemy) { m_pEnemies.push_back(enemy); }

    void DiggerComponent::SetTotalEnemiesForLevel(int total) { m_totalEnemiesForLevel = total; }
    int DiggerComponent::GetTotalEnemiesForLevel() const { return m_totalEnemiesForLevel; }
}
