#ifndef DIGGER_COMPONENT_H
#define DIGGER_COMPONENT_H
#include "Component.h"
#include "Subject.h"
#include "GameObject.h"
#include <vector>
#include <glm/vec2.hpp>

#include <memory>

namespace dae
{
    class DiggerState;
    class HighScoreManager;

    class DiggerComponent : public Component
    {
    public:
        DiggerComponent(GameObject* owner);
        ~DiggerComponent() override;

        void Update(float deltaTime) override;
        void Die();

        void AddObserver(Observer* obs) { m_Subject.AddObserver(obs); }
        void SetOtherPlayer(GameObject* pOther) { m_pOtherPlayer = pOther; }
        void SetDiamonds(const std::vector<GameObject*>& diamonds) { m_pDiamonds = diamonds; }
        void SetGoldBags(const std::vector<GameObject*>& bags) { m_pGoldBags = bags; }

        const std::vector<GameObject*>& GetDiamonds() const { return m_pDiamonds; }
        const std::vector<GameObject*>& GetGoldBags() const { return m_pGoldBags; }

        GameObject* GetOtherPlayer() const { return m_pOtherPlayer; }
        Subject& GetSubject() { return m_Subject; }

        int GetLives() const { return m_Lives; }
        void SetLives(int lives) { m_Lives = lives; m_Subject.Notify(make_sdbm_hash("PlayerDied"), m_Lives); }
        int GetTotalScore() const { return m_TotalScore; }
        void SetTotalScore(int score) { 
            m_TotalScore = score; 
            m_Subject.Notify(make_sdbm_hash("ScoreReset"), score); 
        }

        // High-score manager
        void SetHighScoreManager(HighScoreManager* pMgr) { m_pHighScoreManager = pMgr; }
        HighScoreManager* GetHighScoreManager() const { return m_pHighScoreManager; }

        void ChangeState(DiggerState* newState);

        void SetDesiredDirection(const glm::vec2& dir) { m_DesiredDirection = dir; }
        glm::vec2 GetDesiredDirection() const { return m_DesiredDirection; }
        void SetCurrentDirection(const glm::vec2& dir) { m_CurrentDirection = dir; }
        glm::vec2 GetCurrentDirection() const { return m_CurrentDirection; }

        void SetSpawnPos(const glm::vec2& pos) { m_SpawnPos = pos; }
        glm::vec2 GetSpawnPos() const { return m_SpawnPos; }

        bool IsDead() const { return m_IsDead; }
        void SetDead(bool dead) { m_IsDead = dead; }

        bool IsLevelComplete() const { return m_IsLevelComplete; }
        void SetLevelComplete(bool complete) { m_IsLevelComplete = complete; }

        void Shoot();
        void SetLastFacedDirection(const glm::vec2& dir) { m_LastFacedDirection = dir; }
        glm::vec2 GetLastFacedDirection() const { return m_LastFacedDirection; }
        float GetFireballCooldown() const { return m_FireballCooldown; }
        float GetShootAnimTimer() const { return m_ShootAnimTimer; }

        void SetIsPlayerOne(bool isP1);
        bool IsPlayerOne() const;

        void AwardPoints(int points);
        void AddEmeraldToCombo();
        void ResetEmeraldCombo() { m_ConsecutiveEmeralds = 0; }

        void ActivateBonusMode();
        bool IsInBonusMode() const;

        void SetEnemies(const std::vector<GameObject*>& enemies) { m_pEnemies = enemies; }
        const std::vector<GameObject*>& GetEnemies() const { return m_pEnemies; }
        void AddEnemy(GameObject* enemy) { m_pEnemies.push_back(enemy); }

        // total enemies the level will ever spawn — set by LevelTransitionManager
        void SetTotalEnemiesForLevel(int total) { m_TotalEnemiesForLevel = total; }
        int GetTotalEnemiesForLevel() const { return m_TotalEnemiesForLevel; }

    private:
        Subject m_Subject;
        int m_Lives = 4; // 1 active + 3 reserve (arcade-accurate)

        GameObject* m_pOtherPlayer{ nullptr };
        std::vector<GameObject*> m_pDiamonds{};
        std::vector<GameObject*> m_pGoldBags{};

        std::unique_ptr<DiggerState> m_pCurrentState{ nullptr };

        glm::vec2 m_DesiredDirection{ 0, 0 };
        glm::vec2 m_CurrentDirection{ 0, 0 };
        glm::vec2 m_SpawnPos{ 0, 0 };
        bool m_IsDead{ false };
        bool m_IsLevelComplete{ false };

		glm::vec2 m_LastFacedDirection{ 1, 0 }; // right by default
        float m_FireballCooldown{ 0.0f };
        float m_ShootAnimTimer{ 0.0f };
        bool m_IsPlayerOne{ true };

        int m_TotalScore{ 0 };
        bool m_HasGottenExtraLife{ false };
        int m_ConsecutiveEmeralds{ 0 };

        std::vector<GameObject*> m_pEnemies{};
        int m_TotalEnemiesForLevel{ 0 }; // set per-level by LevelTransitionManager

        HighScoreManager* m_pHighScoreManager{ nullptr }; // not owned
    };
}
#endif