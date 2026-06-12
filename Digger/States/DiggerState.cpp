#include "DiggerState.h"
#include "AudioDefinitions.h"
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
    // Centralized Movement Calculation:
    // We calculate grid-snapping and velocity here rather than inside the distinct states.
    // This ensures both Normal and Bonus modes share the exact same movement physics.
    static glm::vec2 ApplyDiggerMovement(DiggerComponent* digger, float deltaTime)
    {
        // Stop hidden/dead players from clamping back onto the screen!
        if (digger->GetLives() <= 0) return digger->GetOwner()->GetTransform().GetPosition();

        auto myPos = digger->GetOwner()->GetTransform().GetPosition();
        glm::vec2 desiredDir = digger->GetDesiredDirection();
        glm::vec2 currentDir = digger->GetCurrentDirection();

        float gridSize = LevelManager::GetInstance().GetGridSize();
        float offsetY = LevelManager::GetInstance().GetOffsetY();

        float centerX = std::round(myPos.x / gridSize) * gridSize;
        float centerY = std::round((myPos.y - offsetY) / gridSize) * gridSize + offsetY;

        float distToCenter = glm::distance(glm::vec2(myPos.x, myPos.y), glm::vec2(centerX, centerY));

// MOVEMENT
        // Snap to the exact center of a grid tile before allowing the player to turn or dig a new tunnel.
        if (distToCenter < 2.0f)
        {
            // Only carve through dirt if playing natively or as Player 1 in versus
            if (digger->IsPlayerOne() || LevelManager::GetInstance().GetGameMode() != GameMode::Versus) 
            {
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

        // Clamp the player strictly inside the outer walls of the level array
        if (newX < 0) newX = 0;
        if (newX > maxX) newX = maxX;
        if (newY < offsetY) newY = offsetY;
        if (newY > maxY) newY = maxY;

        // In Versus Mode, Player 2 is not allowed to dig through solid dirt. They bounce off it.
        if (!digger->IsPlayerOne() && LevelManager::GetInstance().GetGameMode() == GameMode::Versus)
        {
            if (!LevelManager::GetInstance().IsDug(newX, newY)) 
            {
                newX = centerX; // Snap back to the valid grid intersection!
                newY = centerY;
                digger->SetCurrentDirection({ 0, 0 }); // Freezes momentum so they can turn around!
            }
        }

        digger->GetOwner()->SetLocalPosition(newX, newY);

        if (glm::length(currentDir) > 0) digger->SetLastFacedDirection(currentDir);

        glm::vec2 facing = digger->GetLastFacedDirection();
        std::string prefix = "VR"; // right
        if (facing.x < 0) prefix = "VL"; // left
        else if (facing.y < 0) prefix = "VU"; // up
        else if (facing.y > 0) prefix = "VD"; // down

        std::string frame = "1";

        // Cycle animation frames (1 -> 2 -> 3) based on absolute engine time
        if (glm::length(currentDir) > 0)
        {
            frame = std::to_string((SDL_GetTicks() / 100) % 3 + 1);
        }

        // Override normal animation with the open mouth frame if currently shooting
        if (digger->GetShootAnimTimer() > 0.0f)
        {
            frame = "3";
        }

        // "X" suffix appended to the texture path visually indicates the fireball is on cooldown
        std::string suffix = (digger->GetFireballCooldown() > 0.0f) ? "X" : "";

        if (auto render = digger->GetOwner()->GetComponent<RenderComponent>())
        {
            // Use completely different texture logic if this is the enemy player in Versus mode
            if (!digger->IsPlayerOne() && LevelManager::GetInstance().GetGameMode() == GameMode::Versus) 
            {
                std::string nobFrame = std::to_string((SDL_GetTicks() / 150) % 3 + 1);
                render->SetTexture("PNG/Enemy/VNOB" + nobFrame + ".png");
            }
            else 
            {
                render->SetTexture("PNG/Digger/" + prefix + "DIG" + frame + suffix + ".png");
            }
            render->SetFlip(false); // disable flip
        }

        return glm::vec2{ newX, newY }; // return new position for collision checks
    }

    // normal state
    void DiggerNormalState::OnEnter(DiggerComponent* digger)
    {
        digger->SetDead(false); // Wake up the enemies!
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
        // Player 1 handles diamond pickups. Player 2 (enemy) ignores them.
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
                    ServiceLocator::GetSoundSystem().Play(AudioDefinitions::PICK_UP, 0.5f);
                }
            }
        }

        // Gold Bag pushing logic
        for (auto& bag : digger->GetGoldBags())
        {
            if (!bag || bag->IsMarkedForDestroy()) continue;
            auto bagPos = bag->GetTransform().GetPosition();

            bool touchingX = std::abs(newX - bagPos.x) < 32.0f;
            bool touchingY = std::abs(newY - bagPos.y) < 32.0f;

            if (touchingX && touchingY)
            {
                auto bagComp = bag->GetComponent<GoldBagComponent>();
                
                // If it is broken gold, pick it up for a massive score boost
                if (bagComp && bagComp->IsBroken())
                {
                    bag->MarkForDestroy();
                    digger->AwardPoints(500);
                    digger->ResetEmeraldCombo();
                    ServiceLocator::GetSoundSystem().Play(AudioDefinitions::PICK_UP, 0.5f);
                }
                else
                {
                    // If it is unbroken, push it forward relative to Digger's movement vector
                    glm::vec2 currentDir = digger->GetCurrentDirection();
                    float oldDist = glm::distance(glm::vec2(myPos.x, myPos.y), glm::vec2(bagPos.x, bagPos.y));
                    float newDist = glm::distance(glm::vec2(newX, newY), glm::vec2(bagPos.x, bagPos.y));

                    if (newDist < oldDist)
                    {
                        float deltaX = newX - myPos.x;
                        float deltaY = newY - myPos.y;

                        bag->SetLocalPosition(bagPos.x + deltaX, bagPos.y + deltaY);

                        // Chain pushing: if we push a bag into another bag, shift the second bag forward too!
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
            // Check collisions with all AI enemies
            for (auto& enemy : digger->GetEnemies())
            {
                if (!enemy || enemy->IsMarkedForDestroy()) continue;
                auto ePos = enemy->GetTransform().GetPosition();

                if (glm::distance(glm::vec2{ newX, newY }, glm::vec2{ ePos.x, ePos.y }) < 20.f)
                {
                    // Transition - Switch to Dead state immediately upon collision
                    return new DiggerDeadState();
                }
            }
        }

        if (digger->IsPlayerOne() && LevelManager::GetInstance().GetGameMode() == GameMode::Versus)
        {
            // Check collision with the physical Player 2 in Versus Mode
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
        // Check win conditions.
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

            // Transition: Progress to the next level if either win condition is satisfied
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
        ServiceLocator::GetSoundSystem().PlaySfx(AudioDefinitions::BONUS, 1.0f);

        if (auto render = digger->GetOwner()->GetComponent<RenderComponent>())
        {
            render->SetTexture("PNG/Digger/VRDIG1X.png");
        }

        // DESIGN PATTERN - Observer Pattern
        // Fire an event alerting listeners that Bonus mode began. The Map Manager listens to this to dynamically change the level palette without Digger needing a direct reference to it.
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
                    ServiceLocator::GetSoundSystem().Play(AudioDefinitions::PICK_UP, 0.5f);
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
                    ServiceLocator::GetSoundSystem().Play(AudioDefinitions::PICK_UP, 0.5f);
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

// ALTERED RULE: In bonus mode, touching an enemy eats it for massive points instead of dying
        for (auto& enemy : digger->GetEnemies())
        {
            if (!enemy || enemy->IsMarkedForDestroy()) continue;
            auto ePos = enemy->GetTransform().GetPosition();
            if (glm::distance(glm::vec2(newX, newY), glm::vec2(ePos.x, ePos.y)) < 25.f)
            {
                enemy->MarkForDestroy();
                digger->AwardPoints(250);
                ServiceLocator::GetSoundSystem().Play(AudioDefinitions::KILL_ENEMY, 0.5f);
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
                        // In bonus mode, Player 1 eats Player 2
                        p2Digger->ChangeState(new DiggerDeadState()); // Trigger death state!
                        digger->AwardPoints(250);
                        ServiceLocator::GetSoundSystem().Play(AudioDefinitions::KILL_ENEMY, 0.5f);
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

        // Transition - Revert to Normal state when the timer expires
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
        ServiceLocator::GetSoundSystem().PlaySfx(AudioDefinitions::DEATH, 1.0f);

        if (auto render = digger->GetOwner()->GetComponent<RenderComponent>())
        {
            render->SetTexture("PNG/Grave/VGRAVE1.png");
        }
    }

    DiggerState* DiggerDeadState::Update(DiggerComponent* digger, float deltaTime)
    {
        // Play out the grave fading animation frames sequentially
        m_AnimTimer += deltaTime;
        if (m_AnimTimer > 0.4f && m_CurrentFrame < 5)
        {
            m_AnimTimer -= 0.4f;
            m_CurrentFrame++;
            if (auto render = digger->GetOwner()->GetComponent<RenderComponent>())
            {
                render->SetTexture("PNG/Grave/VGRAVE" + std::to_string(m_CurrentFrame) + ".png");
            }
        }

        m_RespawnTimer -= deltaTime;
        if (m_RespawnTimer <= 0.0f)
        {
            // Transition - If lives remain, safely respawn at the origin. Otherwise, Game Over.
            if (digger->GetLives() > 0)
            {
                digger->SetDead(false);
                digger->SetDesiredDirection(glm::vec2{ 0,0 });
                digger->SetCurrentDirection(glm::vec2{ 0,0 });
                digger->GetOwner()->SetLocalPosition(digger->GetSpawnPos().x, digger->GetSpawnPos().y);

                // cut the death sound and resume looping main music after respawn
                ServiceLocator::GetSoundSystem().StopSfx();
                ServiceLocator::GetSoundSystem().ResumeMusic();

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
        // Intercept Game Over logic to securely transmit the final score to the HighScoreManager
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
            // Move Digger off-screen so they don't linger while the UI resolves
            digger->GetOwner()->SetLocalPosition(-1000.f, -1000.f);
        }
    }

    DiggerState* DiggerGameOverState::Update(DiggerComponent* digger, float deltaTime)
    {
        // Prevent the dead state from hijacking the scene if a reset is pending!
        if (LevelManager::GetInstance().NeedsGameReset()) return nullptr;

        m_Timer += deltaTime;
        if (m_Timer >= 2.0f)
        {
            auto& scenes = SceneManager::GetInstance();
            Scene* gameOverScene = scenes.GetScene(3); // Menu=0, Score=1, Game=2, GameOver=3

            auto mode = LevelManager::GetInstance().GetGameMode();

            // Evaluate end-game rules based on current GameMode
            if (mode == GameMode::SinglePlayer)
            {
                if (gameOverScene) scenes.SetActiveScene(gameOverScene);
            }
            else if (mode == GameMode::CoOp)
            {
                // In Co-Op, game over only triggers when BOTH players are fully dead
                auto other = digger->GetOtherPlayer();
                if (!other || other->IsMarkedForDestroy())
                {
                    if (gameOverScene) scenes.SetActiveScene(gameOverScene);
                }
                else
                {
                    auto otherDigger = other->GetComponent<DiggerComponent>();
                    if (otherDigger && otherDigger->GetLives() <= 0)
                    {
                        if (gameOverScene) scenes.SetActiveScene(gameOverScene);
                    }
                }
            }
            else if (mode == GameMode::Versus)
            {
                // Assign explicit winner text to the UI system
                if (digger->IsPlayerOne())
                {
                    LevelManager::GetInstance().SetWinnerText("PLAYER 2 WINS!");
                }
                else
                {
                    LevelManager::GetInstance().SetWinnerText("PLAYER 1 WINS!");
                }
                if (gameOverScene) scenes.SetActiveScene(gameOverScene);
            }
        }
        return nullptr;
    }

    // level complete state
    void DiggerLevelCompleteState::OnEnter(DiggerComponent* digger)
    {
        digger->SetLevelComplete(true);
        // Halt physical movement completely during the fade out
        digger->SetDesiredDirection(glm::vec2{ 0,0 });
        digger->SetCurrentDirection(glm::vec2{ 0,0 });

        ServiceLocator::GetSoundSystem().PauseMusic();
        ServiceLocator::GetSoundSystem().PlaySfx(AudioDefinitions::NEXT_LEVEL, 0.5f);

        // Dynamically spawn a pure UI component to overlay the active screen and fade it 4s to black
        auto fadeObj = std::make_unique<GameObject>();
        fadeObj->AddComponent<FadeComponent>(4.0f);
        fadeObj->SetZIndex(20);
        SceneManager::GetInstance().GetActiveScene()->Add(std::move(fadeObj));
    }

    DiggerState* DiggerLevelCompleteState::Update(DiggerComponent* digger, float deltaTime)
    {
        m_TransitionTimer -= deltaTime;

        // Transition - After the visual fade finishes, notify the level manager to rebuild the grid
        if (m_TransitionTimer <= 0.0f)
        {
            digger->GetSubject().Notify(make_sdbm_hash("LoadNextLevel"), 0);
            return new DiggerNormalState();
        }
        return nullptr;
    }
}