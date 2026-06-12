#include "EnemyComponent.h"
#include "DiggerComponent.h"
#include "LevelManager.h"
#include "RenderComponent.h"
#include <glm/geometric.hpp>
#include <cmath>
#include <glm/glm.hpp>

namespace dae
{
    EnemyComponent::EnemyComponent(GameObject* owner, DiggerComponent* p1, DiggerComponent* p2)
        : Component(owner)
        , m_p1(p1)
        , m_p2(p2)
    {
        // Enemies always spawn in the Nobbin phase (limited to dug tunnels)
        m_pCurrentState = std::make_unique<NobbinState>();
        m_pCurrentState->OnEnter(this);
    }

    EnemyComponent::~EnemyComponent()
    {
        if (m_pCurrentState) m_pCurrentState->OnExit(this);
    }

    void EnemyComponent::Update(float deltaTime)
    {
        if (m_p1 && m_p1->IsDead()) return;
        if (LevelManager::GetInstance().GetGameMode() != GameMode::SinglePlayer && m_p2 && m_p2->IsDead()) return;

        // Dynamic targeting evaluation: The enemy decides who to chase every frame.
        m_pTarget = nullptr;
        float minDist = 999999.f;
        auto myPos = GetOwner()->GetTransform().GetPosition();

        // Check distance to Player 1
        if (m_p1 && !m_p1->IsDead() && !m_p1->IsLevelComplete())
        {
            auto p1Pos = m_p1->GetOwner()->GetTransform().GetPosition();
            float d = glm::distance(glm::vec2(myPos.x, myPos.y), glm::vec2(p1Pos.x, p1Pos.y));
            m_pTarget = m_p1;
            minDist = d;
        }

        // In Co-Op, check distance to Player 2. The AI is ruthless and aggressively hunts the closest player!
        if (LevelManager::GetInstance().GetGameMode() != GameMode::Versus) {
            if (m_p2 && !m_p2->IsDead() && !m_p2->IsLevelComplete())
            {
                auto p2Pos = m_p2->GetOwner()->GetTransform().GetPosition();
                float d = glm::distance(glm::vec2(myPos.x, myPos.y), glm::vec2(p2Pos.x, p2Pos.y));
                if (!m_pTarget || d < minDist)
                {
                    m_pTarget = m_p2;
                }
            }
        }

        if (!m_pTarget) return;

        // Delegate logic execution down to the active State (Nobbin or Hobbin)
        if (m_pCurrentState)
        {
            std::unique_ptr<EnemyState> newState = m_pCurrentState->Update(this, deltaTime);
            if (newState)
            {
                ChangeState(std::move(newState));
            }
        }
    }

    void EnemyComponent::ChangeState(std::unique_ptr<EnemyState> newState)
    {
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

    void EnemyComponent::MoveAI(float deltaTime)
    {
        if (!m_pTarget) return;

        auto myPos = GetOwner()->GetTransform().GetPosition();
        auto targetPos = m_pTarget->GetOwner()->GetTransform().GetPosition();

        float gridSize = LevelManager::GetInstance().GetGridSize();
        float offsetY = LevelManager::GetInstance().GetOffsetY();

        // Snap to nearest mathematical grid intersection to evaluate pathing options
        float centerX = std::round(myPos.x / gridSize) * gridSize;
        float centerY = std::round((myPos.y - offsetY) / gridSize) * gridSize + offsetY;

        // pathfinding enemies (only calculates when perfectly centered on a tile intersection)
        if (glm::distance(glm::vec2(myPos.x, myPos.y), glm::vec2(centerX, centerY)) < 2.0f)
        {
            glm::vec2 bestDir = m_currentDirection;
            float minTargetDist = 999999.f;
            bool foundPath = false;

            glm::vec2 dirs[4] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
            for (const auto& dir : dirs)
            {
                // Penalize reversing direction heavily so AI doesn't stutter back and forth rapidly
                bool isReverse = (dir.x == -m_currentDirection.x && dir.y == -m_currentDirection.y);

                float checkX = centerX + (dir.x * gridSize);
                float checkY = centerY + (dir.y * gridSize);

                // Prevent AI from pathing out of bounds
                if (checkX < 0 || checkX > 1000 || checkY < offsetY || checkY > 600) continue;

                // nobbins cant dig, so they ignore dirt paths completely
                if (!m_canDig && LevelManager::GetInstance().IsDirtAt(checkX, checkY)) continue;

                float penalty = isReverse ? 10000.f : 0.f;
                float d = glm::distance(glm::vec2(checkX, checkY), glm::vec2(targetPos.x, targetPos.y)) + penalty;

                if (d < minTargetDist)
                {
                    minTargetDist = d;
                    bestDir = dir;
                    foundPath = true;
                }
            }

            if (foundPath) m_currentDirection = bestDir;
            else m_currentDirection = { 0,0 }; // if trapped wait to become a hobbin and dig

            GetOwner()->SetLocalPosition(centerX, centerY);
        }

        // movement
        // Hobbins move slower because they have to push through solid dirt
        float speed = (m_canDig ? 60.0f : 85.0f) * deltaTime;
        float newX = myPos.x + (m_currentDirection.x * speed);
        float newY = myPos.y + (m_currentDirection.y * speed);

        // Hobbins destroy the world geometry as they move!
        if (m_canDig && glm::length(m_currentDirection) > 0) LevelManager::GetInstance().Dig(newX, newY);
        GetOwner()->SetLocalPosition(newX, newY);

        // animation
        m_animTimer += deltaTime;
        if (m_animTimer > 0.15f)
        {
            m_animTimer -= 0.15f;
            m_frame++;
            if (m_frame > 3) m_frame = 1;

            std::string texturePath;
            if (m_canDig)
            {
                std::string prefix = (m_currentDirection.x < 0) ? "PNG/Enemy/VLHOB" : "PNG/Enemy/VRHOB";
                texturePath = prefix + std::to_string(m_frame) + ".png";
            }
            else
            {
                texturePath = "PNG/Enemy/VNOB" + std::to_string(m_frame) + ".png";
            }

            if (auto render = GetOwner()->GetComponent<RenderComponent>())
            {
                render->SetTexture(texturePath);
            }
        }
    }

    void NobbinState::OnEnter(EnemyComponent* enemy)
    {
        enemy->SetCanDig(false);
    }

    std::unique_ptr<EnemyState> NobbinState::Update(EnemyComponent* enemy, float deltaTime)
    {
        m_hobbinTimer -= deltaTime;
        // Trigger the metamorphosis into a Hobbin when the timer runs out!
        if (m_hobbinTimer <= 0.0f) return std::make_unique<HobbinState>();

        enemy->MoveAI(deltaTime);
        return nullptr;
    }

    void HobbinState::OnEnter(EnemyComponent* enemy)
    {
        enemy->SetCanDig(true); // can now eat dirt!
    }

    std::unique_ptr<EnemyState> HobbinState::Update(EnemyComponent* enemy, float deltaTime)
    {
        enemy->MoveAI(deltaTime);
        return nullptr;
    }
}