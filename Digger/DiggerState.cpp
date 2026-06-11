#include "DiggerState.h"
#include "DiggerSounds.h"
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
#include <SDL3/SDL_timer.h>
#include "HighScoreManager.h"
#include <iostream>

namespace dae
{
    static glm::vec2 ApplyDiggerMovement(DiggerComponent* digger, float deltaTime)
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
            if (digger->IsPlayerOne() || LevelManager::GetInstance().GetGameMode() != GameMode::Versus) {
                LevelManager::GetInstance().Dig(centerX, centerY);
            }

            if (glm::length(desiredDir) > 0) currentDir = desiredDir;
            else currentDir = glm::vec2{ 0, 0 };

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

        if (!digger->IsPlayerOne() && LevelManager::GetInstance().GetGameMode() == GameMode::Versus) 
        {
            if (!LevelManager::GetInstance().IsDug(newX, newY)) 
            {
                newX = centerX; // Snap back to the valid grid intersection!
                newY = centerY;
                digger->SetCurrentDirection({0, 0}); // Freezes momentum so they can turn around!
            }
        }

        digger->GetOwner()->SetLocalPosition(newX, newY);

        if (glm::length(currentDir) > 0) digger->SetLastFacedDirection(currentDir);

		// Cannon, Mouth, and Walking animation for digger
        glm::vec2 facing = digger->GetLastFacedDirection();
        std::string prefix = "VR"; // right
        if (facing.x < 0) prefix = "VL"; // left
        else if (facing.y < 0) prefix = "VU"; // up
        else if (facing.y > 0) prefix = "VD"; // down

        std::string frame = "1";

        // 1 -> 2 -> 3 for animation
        if (glm::length(currentDir) > 0) 
        {
            frame = std::to_string((SDL_GetTicks() / 100) % 3 + 1);
        }

        // mouth open
        if (digger->GetShootAnimTimer() > 0.0f) 
        {
            frame = "3";
        }

        // "X" = fireball is on cooldown
        std::string suffix = (digger->GetFireballCooldown() > 0.0f) ? "X" : "";

        if (auto render = digger->GetOwner()->GetComponent<RenderComponent>()) 
        {
            if (!digger->IsPlayerOne() && LevelManager::GetInstance().GetGameMode() == GameMode::Versus) {
                std::string nobFrame = std::to_string((SDL_GetTicks() / 150) % 3 + 1);
                render->SetTexture("PNG/Enemy/VNOB" + nobFrame + ".png");
            } else {
                render->SetTexture("PNG/Digger/" + prefix + "DIG" + frame + suffix + ".png");
            }
            render->SetFlip(false); // disable flip
        }

        return glm::vec2{ newX, newY }; // return new position for collision checks
    }

	// normal state
    void DiggerNormalState::OnEnter(DiggerComponent* digger)
    {
        digger->SetLevelComplete(false);
        if (auto render = digger->GetOwner()->GetComponent<RenderComponent>())
        {
            render->SetTexture("PNG/Digger/VRDIG1X.png");
        }
    }

    DiggerState* DiggerNormalState::Update(DiggerComponent* digger, float deltaTime)
    {
        auto myPos = digger->GetOwner()->GetTransform().GetPosition();

        const glm::vec2 newPos = ApplyDiggerMovement(digger, deltaTime);
        const float newX = newPos.x;
        const float newY = newPos.y;

    // COLLISIONS
        if (digger->IsPlayerOne() || LevelManager::GetInstance().GetGameMode() != GameMode::Versus)
        {
            for (auto& diamond : digger->GetDiamonds())
            {
                if (!diamond || diamond->IsMarkedForDestroy()) continue;

                auto diamondPos = diamond->GetTransform().GetPosition();

                if (glm::distance(glm::vec2{ newX, newY }, glm::vec2{ diamondPos.x, diamondPos.y }) < 20.f)
                {
                    diamond->MarkForDestroy();
                    digger->AwardPoints(25);
                    digger->AddEmeraldToCombo();
                    ServiceLocator::GetSoundSystem().Play(DiggerSounds::PICK_UP, 0.5f);
                }
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
                    digger->AwardPoints(500);
                    digger->ResetEmeraldCombo();
                    ServiceLocator::GetSoundSystem().Play(DiggerSounds::PICK_UP, 0.5f);
                }
                else
                {
                    glm::vec2 currentDir = digger->GetCurrentDirection();
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
        
	// DEATH
        if (digger->IsPlayerOne() || LevelManager::GetInstance().GetGameMode() != GameMode::Versus) 
        {
            for (auto& enemy : digger->GetEnemies())
            {
                if (!enemy || enemy->IsMarkedForDestroy()) continue;
                auto ePos = enemy->GetTransform().GetPosition();

                if (glm::distance(glm::vec2{ newX, newY }, glm::vec2{ ePos.x, ePos.y }) < 20.f)
                {
                    return new DiggerDeadState();
                }
            }
        }

        if (digger->IsPlayerOne() && LevelManager::GetInstance().GetGameMode() == GameMode::Versus) 
        {
            auto p2 = digger->GetOtherPlayer();
            if (p2) 
            {
                auto p2Pos = p2->GetTransform().GetPosition();
                if (glm::distance(glm::vec2(newX, newY), glm::vec2(p2Pos.x, p2Pos.y)) < 20.f) 
                {
                    return new DiggerDeadState();
                }
            }
        }

    // WIN (Collected all the Emeralds OR All Enemies)
        if (!digger->GetDiamonds().empty())
        {
            bool allDiamondsCollected = true;
            for (auto& diamond : digger->GetDiamonds())
            {
                if (diamond && !diamond->IsMarkedForDestroy())
                {
                    allDiamondsCollected = false;
                }
            }

            const int totalForLevel = digger->GetTotalEnemiesForLevel();
            int deadCount = 0;
            for (auto& enemy : digger->GetEnemies())
            {
                if (!enemy || enemy->IsMarkedForDestroy()) deadCount++;
            }
            const bool allSpawned = totalForLevel > 0 && static_cast<int>(digger->GetEnemies().size()) >= totalForLevel;
            const bool allEnemiesDead = allSpawned && (deadCount == static_cast<int>(digger->GetEnemies().size()));

            if (allDiamondsCollected || allEnemiesDead)
            {
                return new DiggerLevelCompleteState();
            }
        }

        return nullptr;
    }

	// bonus state
    void DiggerBonusState::OnEnter(DiggerComponent* digger)
    {
		// pause other music
        ServiceLocator::GetSoundSystem().PauseMusic();

		// sound 1 = bonus.wav (Rossini / William Tell)
        ServiceLocator::GetSoundSystem().PlaySfx(DiggerSounds::BONUS, 1.0f);

        if (auto render = digger->GetOwner()->GetComponent<RenderComponent>())
        {
            render->SetTexture("PNG/Digger/VRDIG1X.png");
        }

		// fire the event — the map manager owns the visual response
        digger->GetSubject().Notify(make_sdbm_hash("BonusModeStart"), 15);
    }

    void DiggerBonusState::OnExit(DiggerComponent* digger)
    {
		// stop bonus music
        ServiceLocator::GetSoundSystem().StopSfx();
		// resume main music
        ServiceLocator::GetSoundSystem().ResumeMusic();

		// tell the map manager to restore the normal palette
        digger->GetSubject().Notify(make_sdbm_hash("BonusModeEnd"), 0);
    }

    DiggerState* DiggerBonusState::Update(DiggerComponent* digger, float deltaTime)
    {
        auto myPos = digger->GetOwner()->GetTransform().GetPosition();

        // run the shared movement + animation logic so Digger is not frozen
        const glm::vec2 newPos = ApplyDiggerMovement(digger, deltaTime);
        const float newX = newPos.x;
        const float newY = newPos.y;

    // COLLISIONS
        if (digger->IsPlayerOne() || LevelManager::GetInstance().GetGameMode() != GameMode::Versus)
        {
            for (auto& diamond : digger->GetDiamonds())
            {
                if (!diamond || diamond->IsMarkedForDestroy()) continue;

                auto diamondPos = diamond->GetTransform().GetPosition();

                if (glm::distance(glm::vec2{ newX, newY }, glm::vec2{ diamondPos.x, diamondPos.y }) < 20.f)
                {
                    diamond->MarkForDestroy();
                    digger->AwardPoints(25);
                    digger->AddEmeraldToCombo();
                    ServiceLocator::GetSoundSystem().Play(DiggerSounds::PICK_UP, 0.5f);
                }
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
                    digger->AwardPoints(500);
                    digger->ResetEmeraldCombo();
                    ServiceLocator::GetSoundSystem().Play(DiggerSounds::PICK_UP, 0.5f);
                }
                else
                {
                    glm::vec2 currentDir = digger->GetCurrentDirection();
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

        // in bonus mode, touching an enemy eats it (250 pts) instead of dying
        for (auto& enemy : digger->GetEnemies())
        {
            if (!enemy || enemy->IsMarkedForDestroy()) continue;
            auto ePos = enemy->GetTransform().GetPosition();
            if (glm::distance(glm::vec2(newX, newY), glm::vec2(ePos.x, ePos.y)) < 25.f)
            {
                enemy->MarkForDestroy();
                digger->AwardPoints(250);
                ServiceLocator::GetSoundSystem().Play(DiggerSounds::KILL_ENEMY, 0.5f);
            }
        }

        if (digger->IsPlayerOne() && LevelManager::GetInstance().GetGameMode() == GameMode::Versus) 
        {
            auto p2 = digger->GetOtherPlayer();
            if (p2 && !p2->IsMarkedForDestroy()) 
            {
                auto p2Digger = p2->GetComponent<DiggerComponent>();
                if (p2Digger && !p2Digger->IsDead()) 
                {
                    auto p2Pos = p2->GetTransform().GetPosition();
                    if (glm::distance(glm::vec2(newX, newY), glm::vec2(p2Pos.x, p2Pos.y)) < 25.f) 
                    {
                        p2Digger->ChangeState(new DiggerDeadState()); // Trigger death state!
                        digger->AwardPoints(250);
                        ServiceLocator::GetSoundSystem().Play(DiggerSounds::KILL_ENEMY, 0.5f);
                    }
                }
            }
        }

    // WIN (Collected all the Emeralds OR All Enemies)
        if (!digger->GetDiamonds().empty())
        {
            bool allDiamondsCollected = true;
            for (auto& diamond : digger->GetDiamonds())
            {
                if (diamond && !diamond->IsMarkedForDestroy())
                {
                    allDiamondsCollected = false;
                }
            }

            const int totalForLevel = digger->GetTotalEnemiesForLevel();
            int deadCount = 0;
            for (auto& enemy : digger->GetEnemies())
            {
                if (!enemy || enemy->IsMarkedForDestroy()) deadCount++;
            }
            const bool allSpawned = totalForLevel > 0 && static_cast<int>(digger->GetEnemies().size()) >= totalForLevel;
            const bool allEnemiesDead = allSpawned && (deadCount == static_cast<int>(digger->GetEnemies().size()));

            if (allDiamondsCollected || allEnemiesDead)
            {
                return new DiggerLevelCompleteState();
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

		// pause music so death sound plays exclusively
        ServiceLocator::GetSoundSystem().PauseMusic();
        ServiceLocator::GetSoundSystem().PlaySfx(DiggerSounds::DEATH, 1.0f);

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

				// cut the death sound and resume looping main music after respawn
                ServiceLocator::GetSoundSystem().StopSfx();
                ServiceLocator::GetSoundSystem().ResumeMusic();

                if (LevelManager::GetInstance().GetGameMode() == GameMode::Versus && digger->IsPlayerOne()) {
                    auto other = digger->GetOtherPlayer();
                    if (other) {
                        auto otherDigger = other->GetComponent<DiggerComponent>();
                        if (otherDigger) other->SetLocalPosition(otherDigger->GetSpawnPos().x, otherDigger->GetSpawnPos().y);
                    }
                }

                return new DiggerNormalState();
            }
            else
            {
                return new DiggerGameOverState();
            }
        }

        return nullptr;
    }

	// game over state
    void DiggerGameOverState::OnEnter(DiggerComponent* digger)
    {
        if (digger)
        {
            HighScoreManager* mgr = digger->GetHighScoreManager();
            if (mgr)
            {
                if (LevelManager::GetInstance().GetGameMode() != GameMode::Versus)
                {
                    mgr->SaveScore(digger->GetTotalScore());
                    std::cout << "Score saved: " << digger->GetTotalScore() << "\n";
                }
            }
        }
    }

    DiggerState* DiggerGameOverState::Update(DiggerComponent* /*digger*/, float /*deltaTime*/)
    {
        return nullptr; // absorb all updates — player is frozen
    }

	// level complete state
    void DiggerLevelCompleteState::OnEnter(DiggerComponent* digger)
    {
        digger->SetLevelComplete(true);
        digger->SetDesiredDirection(glm::vec2{ 0,0 });
        digger->SetCurrentDirection(glm::vec2{ 0,0 });

        ServiceLocator::GetSoundSystem().PauseMusic();
        ServiceLocator::GetSoundSystem().PlaySfx(DiggerSounds::NEXT_LEVEL, 0.5f);

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