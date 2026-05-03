#include "DiggerState.h"
#include "DiggerComponent.h"
#include "RenderComponent.h"
#include "ServiceLocator.h"
#include "LevelManager.h"
#include "GoldBagComponent.h"
#include <glm/glm.hpp>
#include <glm/geometric.hpp>

namespace dae
{
	// normal state
    void DiggerNormalState::OnEnter(DiggerComponent* digger)
    {
        if (auto render = digger->GetOwner()->GetComponent<RenderComponent>())
        {
            render->SetTexture("PNG/Digger/VRDIG1X.png");
        }
    }

    DiggerState* DiggerNormalState::Update(DiggerComponent* digger, float deltaTime)
    {
        auto myPos = digger->GetOwner()->GetTransform().GetPosition();
        glm::vec2 desiredDir = digger->GetDesiredDirection();
        glm::vec2 currentDir = digger->GetCurrentDirection();

        float gridSize = LevelManager::GetInstance().GetGridSize();
        float offsetY = LevelManager::GetInstance().GetOffsetY();

        float centerX = std::round(myPos.x / gridSize) * gridSize;
        float centerY = std::round((myPos.y - offsetY) / gridSize) * gridSize + offsetY;

        float distToCenter = glm::distance(glm::vec2(myPos.x, myPos.y), glm::vec2(centerX, centerY));

        // MOVEMENT
        if (distToCenter < 2.0f)
        {
            LevelManager::GetInstance().Dig(centerX, centerY);

            if (glm::length(desiredDir) > 0) {
                currentDir = desiredDir;
            }
            else {
                currentDir = glm::vec2{ 0, 0 };
            }

            digger->SetCurrentDirection(currentDir);
            digger->GetOwner()->SetLocalPosition(centerX, centerY);
        }

        float speed = 125.0f * deltaTime;
        float newX = myPos.x + (currentDir.x * speed);
        float newY = myPos.y + (currentDir.y * speed);

        float maxX = 25.0f * gridSize;
        float maxY = offsetY + (13.0f * gridSize);

        if (newX < 0) newX = 0;
        if (newX > maxX) newX = maxX;
        if (newY < offsetY) newY = offsetY;
        if (newY > maxY) newY = maxY;

        digger->GetOwner()->SetLocalPosition(newX, newY);

        if (currentDir.x != 0.0f) {
            if (auto render = digger->GetOwner()->GetComponent<RenderComponent>()) {
                render->SetFlip(currentDir.x < 0);
            }
        }

        // COLLISIONS
        for (auto& diamond : digger->GetDiamonds())
        {
            if (!diamond || diamond->IsMarkedForDestroy()) continue;
            auto diamondPos = diamond->GetTransform().GetPosition();
            if (glm::distance(glm::vec2{ newX, newY }, glm::vec2{ diamondPos.x, diamondPos.y }) < 20.f)
            {
                diamond->MarkForDestroy();
                digger->GetSubject().Notify(make_sdbm_hash("DiamondPickedUp"), 25);
            }
        }

        for (auto& bag : digger->GetGoldBags())
        {
            if (!bag || bag->IsMarkedForDestroy()) continue;
            auto bagPos = bag->GetTransform().GetPosition();

            if (glm::distance(glm::vec2{ newX, newY }, glm::vec2{ bagPos.x, bagPos.y }) < 25.f)
            {
                auto bagComp = bag->GetComponent<GoldBagComponent>();
                if (bagComp && bagComp->IsBroken())
                {
                    bag->MarkForDestroy();
                    digger->GetSubject().Notify(make_sdbm_hash("DiamondPickedUp"), 500);
                }
                else if (glm::length(currentDir) > 0)
                {
                    bag->SetLocalPosition(bagPos.x + (currentDir.x * speed), bagPos.y + (currentDir.y * speed));
                }
            }
        }
        return nullptr;
    }

	// bonus state
    void DiggerBonusState::OnEnter(DiggerComponent* digger)
    {
        ServiceLocator::get_sound_system().play(1, 1.0f);

        if (auto render = digger->GetOwner()->GetComponent<RenderComponent>())
        {
			// TODO - bonus texture
            render->SetTexture("PNG/Digger/BONUS_TEXTURE.png");
        }
    }

    void DiggerBonusState::OnExit(DiggerComponent* /*digger*/)
    {
        ServiceLocator::get_sound_system().play(0, 0.5f);
    }

    DiggerState* DiggerBonusState::Update(DiggerComponent* digger, float deltaTime)
    {
        auto myPos = digger->GetOwner()->GetTransform().GetPosition();

        if (auto otherPlayer = digger->GetOtherPlayer())
        {
            auto otherPos = otherPlayer->GetTransform().GetPosition();
            if (glm::distance(glm::vec2(myPos.x, myPos.y), glm::vec2(otherPos.x, otherPos.y)) < 25.f)
            {
                otherPlayer->MarkForDestroy();
                digger->GetSubject().Notify(make_sdbm_hash("DiamondPickedUp"), 500);
            }
        }

        m_BonusTimer -= deltaTime;
        if (m_BonusTimer <= 0.0f)
        {
            return new DiggerNormalState();
        }

        return nullptr;
    }

	// death state
    void DiggerDeadState::OnEnter(DiggerComponent* digger)
    {
        digger->SetDead(true);
        digger->Die();

        if (auto render = digger->GetOwner()->GetComponent<RenderComponent>())
        {
            render->SetTexture("PNG/Grave/VGRAVE1.png");
        }
    }

    DiggerState* DiggerDeadState::Update(DiggerComponent* digger, float deltaTime)
    {
        m_AnimTimer += deltaTime;
        if (m_AnimTimer > 0.4f && m_CurrentFrame < 5)
        {
            m_AnimTimer -= 0.4f;
            m_CurrentFrame++;
            if (auto render = digger->GetOwner()->GetComponent<RenderComponent>())
            {
                render->SetTexture("PNG/Grave/VGRAVE" + std::to_string(m_CurrentFrame) + ".png");
                auto pos = digger->GetOwner()->GetTransform().GetPosition();
                digger->GetOwner()->SetLocalPosition(pos.x, pos.y - 8.0f);
            }
        }

        m_RespawnTimer -= deltaTime;
        if (m_RespawnTimer <= 0.0f)
        {
            if (digger->GetLives() > 0)
            {
                digger->SetDead(false);
                digger->SetDesiredDirection(glm::vec2{ 0,0 });
                digger->SetCurrentDirection(glm::vec2{ 0,0 });
                digger->GetOwner()->SetLocalPosition(digger->GetSpawnPos().x, digger->GetSpawnPos().y);

                return new DiggerNormalState();
            }
            else
            {
                return new DiggerGameOverState();
            }
        }

        return nullptr;
    }
}