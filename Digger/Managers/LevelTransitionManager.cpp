#include "LevelTransitionManager.h"

#include "SceneManager.h"
#include "Scene.h"
#include "LevelManager.h"
#include "DiggerComponent.h"
#include "GoldBagComponent.h"
#include "EnemySpawnerComponent.h"
#include "RenderComponent.h"
#include "CherryComponent.h"
#include "ServiceLocator.h"
#include "UIObservers.h"
#include "AudioDefinitions.h"
#include "DiggerState.h"
#include "HighScoreManager.h"

#include <SDL3/SDL.h>
#include <string>

namespace dae
{
    LevelTransitionManager::LevelTransitionManager(GameObject* owner, Scene* scene, DiggerComponent* p1, DiggerComponent* p2, GameObject* p2Label, GameObject* p2Lives, GameObject* scoreUI1, LivesSpriteDisplayComponent* p2LivesUI)
        : Component(owner)
        , m_pScene(scene)
        , m_p1(p1)
        , m_p2(p2)
        , m_p2Label(p2Label)
        , m_p2Lives(p2Lives)
        , m_pScoreUI1(scoreUI1)
        , m_p2LivesUI(p2LivesUI)
    {
    }

    void LevelTransitionManager::Update(float deltaTime)
    {
        // Wait to load the first level until this scene actually becomes the active one!
        if (!m_HasLoadedFirstLevel && SceneManager::GetInstance().GetActiveScene() == m_pScene)
        {
            m_HasLoadedFirstLevel = true;
            LoadLevel(0);
        }

        if (LevelManager::GetInstance().NeedsGameReset() && SceneManager::GetInstance().GetActiveScene() == m_pScene)
        {
            LevelManager::GetInstance().SetNeedsGameReset(false);

            GameMode mode = LevelManager::GetInstance().GetGameMode();

            if (m_p1) { m_p1->SetLives(4); m_p1->SetTotalScore(0); }
            if (m_p2) { 
                if (mode == GameMode::SinglePlayer) m_p2->SetLives(0);
                else { m_p2->SetLives(4); m_p2->SetTotalScore(0); }
            }

            // Update score in the HighScoreManager to 0
            if (m_p1 && m_p1->GetHighScoreManager()) m_p1->GetHighScoreManager()->SetCurrentScore(0);

            m_CurrentLevelIndex = 0;
            LoadLevel(0);
        }

        if (!m_BonusMapActive) return;

        if (m_BonusFlickerPhase)
        {
            m_BonusFlickerTimer += deltaTime;
            m_BonusFlickerInterval += deltaTime;

            if (m_BonusFlickerInterval >= 0.15f)
            {
                m_BonusFlickerInterval -= 0.15f;
                m_BonusLightOn = !m_BonusLightOn;
                ApplyDirtBrightness(m_BonusLightOn);
            }

            if (m_BonusFlickerTimer >= 5.0f)
            {
                // Flicker over — lock to BRIGHT for the rest of bonus time.
                m_BonusFlickerPhase = false;
                ApplyDirtBrightness(true);
            }
        }
        // Solid-bright phase: boost already applied, nothing to do each frame.
    }

    void LevelTransitionManager::OnNotify(EventId eventId, int /*value*/)
    {
        if (eventId == make_sdbm_hash("LoadNextLevel"))
        {
            const uint64_t currentTime = SDL_GetTicks();
            if (currentTime - m_LastLoadTime < 1000) return; // 1-second cooldown
            m_LastLoadTime = currentTime;

            m_CurrentLevelIndex++;
            if (m_CurrentLevelIndex >= LevelManager::GetInstance().GetTotalLevels()) m_CurrentLevelIndex = 0;

            LoadLevel(m_CurrentLevelIndex);
        }
        else if (eventId == make_sdbm_hash("EnemyThresholdReached"))
        {
            // 75% of enemies spawned — show the cherry at the 'B' tile
            if (!m_CherrySpawned && m_CherrySpawnX > 0.0f)
            {
                SpawnCherry();
                m_CherrySpawned = true;
            }
        }
        else if (eventId == make_sdbm_hash("BonusModeStart"))
        {
            // Start the flicker. Begin on NORMAL so the first toggle flashes BRIGHT.
            m_BonusMapActive = true;
            m_BonusFlickerPhase = true;
            m_BonusFlickerTimer = 0.0f;
            m_BonusFlickerInterval = 0.0f;
            m_BonusLightOn = false;
            ApplyDirtBrightness(false);
        }
        else if (eventId == make_sdbm_hash("BonusModeEnd"))
        {
            m_BonusMapActive = false;
            m_BonusFlickerPhase = false;
            ApplyDirtBrightness(false);
        }
    }

    void LevelTransitionManager::LoadLevel(int levelIndex)
    {
        // Clean audio environment for the new level without restarting the main loop from 0
        ServiceLocator::GetSoundSystem().StopSfx();
        ServiceLocator::GetSoundSystem().ResumeMusic();

        // Enforce a strict state reset to destroy any active Bonus Mode and its timer
        if (m_p1) {
            if (m_p1->GetLives() > 0) m_p1->ChangeState(new DiggerNormalState());
            else m_p1->GetOwner()->SetLocalPosition(-1000.f, -1000.f); // Keep them hidden
        }
        if (m_p2) {
            if (m_p2->GetLives() > 0) m_p2->ChangeState(new DiggerNormalState());
            else m_p2->GetOwner()->SetLocalPosition(-1000.f, -1000.f); // Keep them hidden
        }

        // Destroy old entities
        for (auto* bag : m_p1->GetGoldBags()) { if (bag) bag->MarkForDestroy(); }
        for (auto* dia : m_p1->GetDiamonds()) { if (dia) dia->MarkForDestroy(); }
        for (auto* enemy : m_p1->GetEnemies()) { if (enemy) enemy->MarkForDestroy(); }
        for (auto* dirt : m_VisualDirt) { if (dirt) dirt->MarkForDestroy(); }
        for (auto* entity : m_MiscEntities) { if (entity) entity->MarkForDestroy(); }

        m_MiscEntities.clear();
        m_p1->SetGoldBags({});
        m_p1->SetDiamonds({});
        m_p1->SetEnemies({});
        if (m_p2)
        {
            m_p2->SetGoldBags({});
            m_p2->SetDiamonds({});
            m_p2->SetEnemies({});
        }
        m_VisualDirt.clear();

        // Reset and re-initialise the grid
        m_CherrySpawnX = 0.0f;
        m_CherrySpawnY = 0.0f;
        m_CherrySpawned = false;

        m_p1->SetTotalEnemiesForLevel(0);
        if (m_p2) m_p2->SetTotalEnemiesForLevel(0);

        m_BonusMapActive = false;
        m_BonusFlickerPhase = false;

        LevelManager::GetInstance().ClearLevel();
        m_pScene->RequestLevelCleanup(); // Scene cleans up at end of frame
        LevelManager::GetInstance().InitLevel(14, 26);

        GameMode mode = LevelManager::GetInstance().GetGameMode();
        
        // 1. Handle Player 2 Physics/Visuals
        if (mode == GameMode::SinglePlayer && m_p2)
        {
            m_p2->SetLives(0);
            m_p2->GetOwner()->SetLocalPosition(-1000.f, -1000.f); // Safely hide
        }
        else if (mode == GameMode::Versus && m_p2)
        {
            if (auto p2Render = m_p2->GetOwner()->GetComponent<RenderComponent>())
            {
                p2Render->SetTexture("PNG/Enemy/VNOB1.png");
            }
        }

        // 2. Safely toggle HUD visibility via coordinates (Never Destroy!)
        if (mode == GameMode::SinglePlayer)
        {
            if (m_pScoreUI1) m_pScoreUI1->SetLocalPosition(500, 13);
            if (m_p2Label) m_p2Label->SetLocalPosition(-1000, -1000); // Hide P2 Label
            if (m_p2Lives) m_p2Lives->SetLocalPosition(-1000, -1000); // Hide P2 Lives
        }
        else if (mode == GameMode::Versus)
        {
            if (m_pScoreUI1) m_pScoreUI1->SetLocalPosition(-1000, -1000); // Hide Score
            if (m_p2Label) m_p2Label->SetLocalPosition(905, 15);          // Restore P2 Label
            if (m_p2Lives) m_p2Lives->SetLocalPosition(850, 15);          // Restore P2 Lives
            if (m_p2LivesUI) m_p2LivesUI->SetTexture("PNG/Enemy/VNOB1.png");
        }
        else // CoOp
        {
            if (m_pScoreUI1) m_pScoreUI1->SetLocalPosition(500, 13);      // Restore Score
            if (m_p2Label) m_p2Label->SetLocalPosition(905, 15);          // Restore P2 Label
            if (m_p2Lives) m_p2Lives->SetLocalPosition(850, 15);          // Restore P2 Lives
            if (m_p2LivesUI) m_p2LivesUI->SetTexture("PNG/Digger/VRDIG1X.png"); 
        }

        // Parse tile layout
        const auto layout = LevelManager::GetInstance().GetLevelLayout(levelIndex);
        constexpr float tileWidth = 40.0f;
        constexpr float startY = 52.0f;

        const int textureNumber = (levelIndex % 3) + 1;
        const std::string normalTexture = "PNG/Map/VBACK" + std::to_string(textureNumber) + ".png";

        std::vector<GameObject*> newBags;
        std::vector<GameObject*> newDiamonds;
        std::vector<GameObject*> newEnemies;

        // Pass 1: background dirt tiles and hole objects
        for (int row = 0; row < static_cast<int>(layout.size()); ++row)
        {
            for (int col = 0; col < static_cast<int>(layout[row].size()); ++col)
            {
                const float bx = static_cast<float>(col) * tileWidth;
                const float by = startY + (static_cast<float>(row) * tileWidth);

                if (layout[row][col] != ' ')
                {
                    LevelManager::GetInstance().AddDirtTile(col, row);

                    for (int strip = 0; strip < 5; ++strip)
                    {
                        auto tile = std::make_unique<GameObject>();
                        tile->AddComponent<RenderComponent>(normalTexture);
                        tile->SetLocalPosition(bx, by + (static_cast<float>(strip) * 8.0f));
                        tile->SetZIndex(1);
                        m_VisualDirt.push_back(tile.get());
                        m_pScene->Add(std::move(tile));
                    }
                }

                auto holeObj = std::make_unique<GameObject>();
                holeObj->SetLocalPosition(bx, by);
                holeObj->SetZIndex(2);
                LevelManager::GetInstance().RegisterHoleObject(col, row, holeObj.get());
                m_pScene->Add(std::move(holeObj));
            }
        }

        // Pass 2: entities and player spawn points
        for (int row = 0; row < static_cast<int>(layout.size()); ++row)
        {
            for (int col = 0; col < static_cast<int>(layout[row].size()); ++col)
            {
                const char c = layout[row][col];
                const float bx = static_cast<float>(col) * tileWidth;
                const float by = startY + (static_cast<float>(row) * tileWidth);

                if (c == ' ' || c == 'P' || c == 'S' || c == 'E' || c == 'B') LevelManager::GetInstance().Dig(bx, by);

                if (c == 'P')
                {
                    m_p1->SetSpawnPos({ bx, by });
                    if (m_p1->GetLives() > 0) m_p1->GetOwner()->SetLocalPosition(bx, by);
                }
                else if (c == 'S' && m_p2)
                {
                    m_p2->SetSpawnPos({ bx, by });
                    if (m_p2->GetLives() > 0) m_p2->GetOwner()->SetLocalPosition(bx, by);
                }
                else if (c == 'E')
                {
                    constexpr int k_MaxTotal = 4;
                    constexpr int k_MaxConcurrent = 2;

                    auto spawner = std::make_unique<GameObject>();
                    auto* spawnerC = spawner->AddComponent<EnemySpawnerComponent>(m_p1, m_p2, k_MaxTotal, k_MaxConcurrent);

                    m_p1->SetTotalEnemiesForLevel(k_MaxTotal);
                    if (m_p2) m_p2->SetTotalEnemiesForLevel(k_MaxTotal);

                    spawnerC->AddObserver(this); // notified when 75% spawned → SpawnCherry
                    spawner->SetLocalPosition(bx, by);
                    m_MiscEntities.push_back(spawner.get());
                    m_pScene->Add(std::move(spawner));
                }
                else if (c == 'B')
                {
                    // Cherry spawn point — record position, no GameObject yet
                    m_CherrySpawnX = bx;
                    m_CherrySpawnY = by;
                }
                else if (c == 'D')
                {
                    auto diamond = std::make_unique<GameObject>();
                    auto* render = diamond->AddComponent<RenderComponent>("PNG/Money/CEMERALD.png");
                    render->SetScale(2.f);
                    diamond->SetLocalPosition(bx, by);
                    diamond->SetZIndex(3);
                    newDiamonds.push_back(diamond.get());
                    m_pScene->Add(std::move(diamond));
                }
                else if (c == 'C')
                {
                    auto goldBag = std::make_unique<GameObject>();
                    auto* render = goldBag->AddComponent<RenderComponent>("PNG/Money/CSBAG.png");
                    render->SetScale(2.f);
                    goldBag->SetLocalPosition(bx, by);
                    auto* bagComp = goldBag->AddComponent<GoldBagComponent>();
                    bagComp->SetPlayers(m_p1->GetOwner(), m_p2 ? m_p2->GetOwner() : nullptr);
                    goldBag->SetZIndex(4);
                    newBags.push_back(goldBag.get());
                    m_pScene->Add(std::move(goldBag));
                }
            }
        }

        // Register new entities with the digger components
        m_p1->SetGoldBags(newBags);
        m_p1->SetDiamonds(newDiamonds);
        m_p1->SetEnemies(newEnemies);

        if (m_p2)
        {
            m_p2->SetGoldBags(newBags);
            m_p2->SetDiamonds(newDiamonds);
            m_p2->SetEnemies(newEnemies);
        }
    }

    void LevelTransitionManager::SpawnCherry()
    {
        auto cherry = std::make_unique<GameObject>();
        auto* render = cherry->AddComponent<RenderComponent>("PNG/Other/CBONUS.png");
        render->SetScale(2.f);
        cherry->AddComponent<CherryComponent>(m_p1, m_p2);
        cherry->SetLocalPosition(m_CherrySpawnX, m_CherrySpawnY);
        cherry->SetZIndex(6);
        m_MiscEntities.push_back(cherry.get());
        m_pScene->Add(std::move(cherry));
    }

    void LevelTransitionManager::ApplyDirtBrightness(bool enable)
    {
        for (auto* tile : m_VisualDirt)
        {
            if (!tile || tile->IsMarkedForDestroy()) continue;
            if (auto* render = tile->GetComponent<RenderComponent>()) render->SetAdditiveBoost(enable); // default boostAlpha=120 (~47% brighter)
        }
    }
}
