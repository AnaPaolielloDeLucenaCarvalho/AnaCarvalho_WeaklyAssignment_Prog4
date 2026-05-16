#include "DiggerState.h"
#include "DiggerComponent.h"
#include "RenderComponent.h"
#include "ServiceLocator.h"
#include "LevelManager.h"
#include "GoldBagComponent.h"
#include <glm/glm.hpp>
#include <glm/geometric.hpp>
#include "FadeComponent.h"
#include "SceneManager.h"
#include "Scene.h"

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

            if (glm::length(desiredDir) > 0) 
            {
                currentDir = desiredDir;
            }
            else 
            {
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

        if (currentDir.x != 0.0f) 
        {
            if (auto render = digger->GetOwner()->GetComponent<RenderComponent>()) 
            {
                render->SetFlip(currentDir.x < 0);
            }
        }

	// SHOOTING
        digger->GetOwner()->SetLocalPosition(newX, newY);

        if (currentDir.x != 0.0f) 
        {
            if (auto render = digger->GetOwner()->GetComponent<RenderComponent>()) 
            {
                render->SetFlip(currentDir.x < 0);
            }
        }

        if (glm::length(currentDir) > 0) 
        {
            digger->SetLastFacedDirection(currentDir);
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

            bool touchingX = std::abs(newX - bagPos.x) < 32.0f;
            bool touchingY = std::abs(newY - bagPos.y) < 32.0f;

            if (touchingX && touchingY)
            {
                auto bagComp = bag->GetComponent<GoldBagComponent>();
                if (bagComp && bagComp->IsBroken())
                {
                    bag->MarkForDestroy();
                    digger->GetSubject().Notify(make_sdbm_hash("DiamondPickedUp"), 500);
                }
                else
                {
                    float oldDist = glm::distance(glm::vec2(myPos.x, myPos.y), glm::vec2(bagPos.x, bagPos.y));
                    float newDist = glm::distance(glm::vec2(newX, newY), glm::vec2(bagPos.x, bagPos.y));

                    if (newDist < oldDist)
                    {
                        float deltaX = newX - myPos.x;
                        float deltaY = newY - myPos.y;

                        bag->SetLocalPosition(bagPos.x + deltaX, bagPos.y + deltaY);

                        for (int chain = 0; chain < 3; ++chain)
                        {
                            for (auto& b1 : digger->GetGoldBags())
                            {
                                if (!b1 || b1->IsMarkedForDestroy()) continue;
                                auto p1 = b1->GetTransform().GetPosition();

                                for (auto& b2 : digger->GetGoldBags())
                                {
                                    if (b1 == b2 || !b2 || b2->IsMarkedForDestroy()) continue;
                                    auto p2 = b2->GetTransform().GetPosition();

                                    if (std::abs(p1.x - p2.x) < 38.0f && std::abs(p1.y - p2.y) < 38.0f)
                                    {
                                        if (currentDir.x > 0 && p1.x < p2.x) b2->SetLocalPosition(p1.x + 38.0f, p2.y);
                                        else if (currentDir.x < 0 && p1.x > p2.x) b2->SetLocalPosition(p1.x - 38.0f, p2.y);
                                        else if (currentDir.y > 0 && p1.y < p2.y) b2->SetLocalPosition(p2.x, p1.y + 38.0f);
                                        else if (currentDir.y < 0 && p1.y > p2.y) b2->SetLocalPosition(p2.x, p1.y - 38.0f);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        
    // WIN (Collected all the Emeralds)
        if (!digger->GetDiamonds().empty())
        {
            bool allDiamondsCollected = true;
            for (auto& diamond : digger->GetDiamonds())
            {
                if (diamond && !diamond->IsMarkedForDestroy())
                {
                    allDiamondsCollected = false;
                    break;
                }
            }

            if (allDiamondsCollected)
            {
                return new DiggerLevelCompleteState();
            }
        }

        return nullptr;
    }

	// bonus state
    void DiggerBonusState::OnEnter(DiggerComponent* digger)
    {
        ServiceLocator::GetSoundSystem().Play(1, 1.0f);

        if (auto render = digger->GetOwner()->GetComponent<RenderComponent>())
        {
			// TODO - bonus texture
            render->SetTexture("PNG/Digger/BONUS_TEXTURE.png");
        }
    }

    void DiggerBonusState::OnExit(DiggerComponent* /*digger*/)
    {
        ServiceLocator::GetSoundSystem().Play(0, 0.5f);
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

	// level complete state
    void DiggerLevelCompleteState::OnEnter(DiggerComponent* digger)
    {
        digger->SetDesiredDirection(glm::vec2{ 0,0 });
        digger->SetCurrentDirection(glm::vec2{ 0,0 });

        ServiceLocator::GetSoundSystem().Play(2, 0.5f);

		// fade 4s to black
        auto fadeObj = std::make_unique<GameObject>();
        fadeObj->AddComponent<FadeComponent>(4.0f);
        fadeObj->SetZIndex(20);
        SceneManager::GetInstance().GetActiveScene()->Add(std::move(fadeObj));
    }

    DiggerState* DiggerLevelCompleteState::Update(DiggerComponent* digger, float deltaTime)
    {
        m_TransitionTimer -= deltaTime;

        if(m_TransitionTimer <= 0.0f)
        {
            digger->GetSubject().Notify(make_sdbm_hash("LoadNextLevel"), 0);
            return new DiggerNormalState();
        }
        return nullptr;
    }
}