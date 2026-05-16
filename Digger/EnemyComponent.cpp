#include "EnemyComponent.h"
#include "DiggerComponent.h"
#include "LevelManager.h"
#include "RenderComponent.h"
#include <glm/geometric.hpp>
#include <cmath>

namespace dae
{
    EnemyComponent::EnemyComponent(GameObject* owner, DiggerComponent* targetPlayer)
        : Component(owner)
        , m_pTarget(targetPlayer)
    {
        m_pCurrentState = std::make_unique<NobbinState>();
        m_pCurrentState->OnEnter(this);
    }

    EnemyComponent::~EnemyComponent()
    {
        if (m_pCurrentState) m_pCurrentState->OnExit(this);
    }

    void EnemyComponent::Update(float deltaTime)
    {
        if (m_pCurrentState)
        {
            EnemyState* newState = m_pCurrentState->Update(this, deltaTime);
            if (newState)
            {
                ChangeState(newState);
            }
        }
    }

    void EnemyComponent::ChangeState(EnemyState* newState)
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

    void EnemyComponent::MoveAI(float deltaTime)
    {
        if (!m_pTarget) return;

        auto myPos = GetOwner()->GetTransform().GetPosition();
        auto targetPos = m_pTarget->GetOwner()->GetTransform().GetPosition();

        float gridSize = LevelManager::GetInstance().GetGridSize();
        float offsetY = LevelManager::GetInstance().GetOffsetY();

        float centerX = std::round(myPos.x / gridSize) * gridSize;
        float centerY = std::round((myPos.y - offsetY) / gridSize) * gridSize + offsetY;

		// pathfinding enemies
        if (glm::distance(glm::vec2(myPos.x, myPos.y), glm::vec2(centerX, centerY)) < 2.0f)
        {
            glm::vec2 bestDir = m_CurrentDirection;
            float minTargetDist = 999999.f;
            bool foundPath = false;

            glm::vec2 dirs[4] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
            for (auto& dir : dirs)
            {
                bool isReverse = (dir.x == -m_CurrentDirection.x && dir.y == -m_CurrentDirection.y);

                float checkX = centerX + (dir.x * gridSize);
                float checkY = centerY + (dir.y * gridSize);

                if (checkX < 0 || checkX > 1000 || checkY < offsetY || checkY > 600) continue;

                // nobbins cant dig
                if (!m_CanDig && LevelManager::GetInstance().IsDirtAt(checkX, checkY)) continue;

                float penalty = isReverse ? 10000.f : 0.f;
                float d = glm::distance(glm::vec2(checkX, checkY), glm::vec2(targetPos.x, targetPos.y)) + penalty;

                if (d < minTargetDist)
                {
                    minTargetDist = d;
                    bestDir = dir;
                    foundPath = true;
                }
            }

            if (foundPath) m_CurrentDirection = bestDir;
			else m_CurrentDirection = { 0,0 }; // if trapped wiat to become a hobbin and dig

            GetOwner()->SetLocalPosition(centerX, centerY);
        }

		// movement
        float speed = (m_CanDig ? 60.0f : 85.0f) * deltaTime;
        float newX = myPos.x + (m_CurrentDirection.x * speed);
        float newY = myPos.y + (m_CurrentDirection.y * speed);

        if (m_CanDig && glm::length(m_CurrentDirection) > 0) LevelManager::GetInstance().Dig(newX, newY);
        GetOwner()->SetLocalPosition(newX, newY);

		// animation
        m_AnimTimer += deltaTime;
        if (m_AnimTimer > 0.15f)
        {
            m_AnimTimer -= 0.15f;
            m_Frame++;
            if (m_Frame > 3) m_Frame = 1;

            std::string texturePath;
            if (m_CanDig)
            {
                std::string prefix = (m_CurrentDirection.x < 0) ? "PNG/Enemy/VLHOB" : "PNG/Enemy/VRHOB";
                texturePath = prefix + std::to_string(m_Frame) + ".png";
            }
            else
            {
                texturePath = "PNG/Enemy/VNOB" + std::to_string(m_Frame) + ".png";
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

    EnemyState* NobbinState::Update(EnemyComponent* enemy, float deltaTime)
    {
        m_HobbinTimer -= deltaTime;
        if (m_HobbinTimer <= 0.0f) return new HobbinState();

        enemy->MoveAI(deltaTime);
        return nullptr;
    }

    void HobbinState::OnEnter(EnemyComponent* enemy)
    {
        enemy->SetCanDig(true); // can now eat dirt!
    }

    EnemyState* HobbinState::Update(EnemyComponent* enemy, float deltaTime)
    {
        enemy->MoveAI(deltaTime);
        return nullptr;
    }
}