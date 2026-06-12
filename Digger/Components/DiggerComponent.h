#ifndef DIGGER_COMPONENT_H
#define DIGGER_COMPONENT_H
#include "Component.h"
#include "Subject.h"
#include "GameObject.h"
#include <vector>
#include <glm/vec2.hpp>

#include <memory>

// DESIGN PATTERN - State Pattern & Observer Pattern
// Used the State pattern (m_pCurrentState) so the player doesn't rely on a giant switch statement or a dozen booleans to handle Normal/Bonus/Dead logic. Also used Observer pattern by giving Digger a Subject. It can just shout "I died!"  or "I scored!" and the decoupled UI elements update themselves automatically.

namespace dae
{
    class DiggerState;
    class HighScoreManager;

    class DiggerComponent final : public Component
    {
    public:
        explicit DiggerComponent(GameObject* owner);
        ~DiggerComponent() override;

        DiggerComponent(const DiggerComponent&) = delete;
        DiggerComponent(DiggerComponent&&) = delete;
        DiggerComponent& operator=(const DiggerComponent&) = delete;
        DiggerComponent& operator=(DiggerComponent&&) = delete;

        void Render() const override {}
        void Update(float deltaTime) override;
        void Die();

        void AddObserver(Observer* obs);
        void SetOtherPlayer(GameObject* pOther);
        void SetDiamonds(const std::vector<GameObject*>& diamonds);
        void SetGoldBags(const std::vector<GameObject*>& bags);

        const std::vector<GameObject*>& GetDiamonds() const;
        const std::vector<GameObject*>& GetGoldBags() const;

        GameObject* GetOtherPlayer() const;
        Subject& GetSubject();

        int GetLives() const;
        void SetLives(int lives);
        int GetTotalScore() const;
        void SetTotalScore(int score);

        // High-score manager
        void SetHighScoreManager(HighScoreManager* pMgr);
        HighScoreManager* GetHighScoreManager() const;

        void ChangeState(std::unique_ptr<dae::DiggerState> newState);

        void SetDesiredDirection(const glm::vec2& dir);
        glm::vec2 GetDesiredDirection() const;
        void SetCurrentDirection(const glm::vec2& dir);
        glm::vec2 GetCurrentDirection() const;

        void SetSpawnPos(const glm::vec2& pos);
        glm::vec2 GetSpawnPos() const;

        bool IsDead() const;
        void SetDead(bool dead);

        bool IsLevelComplete() const;
        void SetLevelComplete(bool complete);

        void Shoot();
        void SetLastFacedDirection(const glm::vec2& dir);
        glm::vec2 GetLastFacedDirection() const;
        float GetFireballCooldown() const;
        float GetShootAnimTimer() const;

        void SetIsPlayerOne(bool isP1);
        bool IsPlayerOne() const;

        void AwardPoints(int points, bool breaksCombo = true);
        void AddEmeraldToCombo();
        void ResetEmeraldCombo();

        void ActivateBonusMode();
        bool IsInBonusMode() const;

        void SetEnemies(const std::vector<GameObject*>& enemies);
        const std::vector<GameObject*>& GetEnemies() const;
        void AddEnemy(GameObject* enemy);

        // total enemies the level will ever spawn — set by LevelTransitionManager
        void SetTotalEnemiesForLevel(int total);
        int GetTotalEnemiesForLevel() const;

    private:
        Subject m_subject;
        int m_lives = 4; // 1 active + 3 reserve (arcade-accurate)

        GameObject* m_pOtherPlayer{ nullptr };
        std::vector<GameObject*> m_pDiamonds{};
        std::vector<GameObject*> m_pGoldBags{};

        std::unique_ptr<DiggerState> m_pCurrentState{ nullptr };

        glm::vec2 m_desiredDirection{ 0, 0 };
        glm::vec2 m_currentDirection{ 0, 0 };
        glm::vec2 m_spawnPos{ 0, 0 };
        bool m_isDead{ false };
        bool m_isLevelComplete{ false };

        glm::vec2 m_lastFacedDirection{ 1, 0 }; // right by default
        float m_fireballCooldown{ 0.0f };
        float m_shootAnimTimer{ 0.0f };
        bool m_isPlayerOne{ true };

        int m_totalScore{ 0 };
        int m_extraLifeMilestones{ 0 };
        int m_consecutiveEmeralds{ 0 };

        std::vector<GameObject*> m_pEnemies{};
        int m_totalEnemiesForLevel{ 0 }; // set per-level by LevelTransitionManager

        HighScoreManager* m_pHighScoreManager{ nullptr }; // not owned
    };
}
#endif