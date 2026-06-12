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
        if (!m_hasLoadedFirstLevel && SceneManager::GetInstance().GetActiveScene() == m_pScene)
        {
            m_hasLoadedFirstLevel = true;
            LoadLevel(0);
        }

        // Hard Reset - Called when returning to the Main Menu or restarting after a Game Over
        if (LevelManager::GetInstance().NeedsGameReset() && SceneManager::GetInstance().GetActiveScene() == m_pScene)
        {
            LevelManager::GetInstance().SetNeedsGameReset(false);

            GameMode mode = LevelManager::GetInstance().GetGameMode();

            // Re-initialize players to factory defaults
            if (m_p1) 
            { 
                m_p1->SetLives(4); m_p1->SetTotalScore(0); 
            }
            if (m_p2) 
            {
                if (mode == GameMode::SinglePlayer)
                {
                    m_p2->SetLives(0);
                }
                else 
                { 
                    m_p2->SetLives(4); m_p2->SetTotalScore(0); 
                }
            }

            // Update score in the HighScoreManager to 0
            if (m_p1 && m_p1->GetHighScoreManager()) m_p1->GetHighScoreManager()->SetCurrentScore(0);

            m_currentLevelIndex = 0;
            LoadLevel(0);
        }

        // Fast escape if no bonus mode visual effects are required right now
        if (!m_bonusMapActive) return;

        // Visual Flicker Effect - Makes the dirt blocks rapidly switch brightness to simulate an arcade panic state
        if (m_bonusFlickerPhase)
        {
            m_bonusFlickerTimer += deltaTime;
            m_bonusFlickerInterval += deltaTime;

            if (m_bonusFlickerInterval >= 0.15f)
            {
                m_bonusFlickerInterval -= 0.15f;
                m_bonusLightOn = !m_bonusLightOn;
                ApplyDirtBrightness(m_bonusLightOn);
            }

            if (m_bonusFlickerTimer >= 5.0f)
            {
                // Flicker over — lock to BRIGHT for the rest of bonus time.
                m_bonusFlickerPhase = false;
                ApplyDirtBrightness(true);
            }
        }
        // Solid-bright phase: boost already applied, nothing to do each frame.
    }

    void LevelTransitionManager::OnNotify(EventId eventId, int /*value*/)
    {
        if (eventId == make_sdbm_hash("LoadNextLevel"))
        {
            // Enforce a 1-second cooldown using an absolute timestamp so we don't accidentally load 2 levels instantly
            const uint64_t currentTime = SDL_GetTicks();
            if (currentTime - m_lastLoadTime < 1000) return;
            m_lastLoadTime = currentTime;

            // Increment and wrap around to the beginning if we run out of level files
            m_currentLevelIndex++;
            if (m_currentLevelIndex >= LevelManager::GetInstance().GetTotalLevels()) m_currentLevelIndex = 0;

            LoadLevel(m_currentLevelIndex);
        }
        else if (eventId == make_sdbm_hash("EnemyThresholdReached"))
        {
            // 75% of enemies spawned — show the cherry at the 'B' tile
            if (!m_cherrySpawned && m_cherrySpawnX > 0.0f)
            {
                SpawnCherry();
                m_cherrySpawned = true;
            }
        }
        else if (eventId == make_sdbm_hash("BonusModeStart"))
        {
            // Start the flicker. Begin on NORMAL so the first toggle flashes BRIGHT.
            m_bonusMapActive = true;
            m_bonusFlickerPhase = true;
            m_bonusFlickerTimer = 0.0f;
            m_bonusFlickerInterval = 0.0f;
            m_bonusLightOn = false;
            ApplyDirtBrightness(false);
        }
        else if (eventId == make_sdbm_hash("BonusModeEnd"))
        {
            m_bonusMapActive = false;
            m_bonusFlickerPhase = false;
            ApplyDirtBrightness(false);
        }
    }

    void LevelTransitionManager::LoadLevel(int levelIndex)
    {
        // Clean audio environment for the new level without restarting the main loop from 0
        ServiceLocator::GetSoundSystem().StopSfx();
        ServiceLocator::GetSoundSystem().ResumeMusic();

        // Enforce a strict state reset to destroy any active Bonus Mode and its timer
        if (m_p1) 
        {
            if (m_p1->GetLives() > 0) m_p1->ChangeState(std::make_unique<DiggerNormalState>());
            else m_p1->GetOwner()->SetLocalPosition(-1000.f, -1000.f); // Keep them hidden
        }
        if (m_p2) 
        {
            if (m_p2->GetLives() > 0) m_p2->ChangeState(std::make_unique<DiggerNormalState>());
            else m_p2->GetOwner()->SetLocalPosition(-1000.f, -1000.f); // Keep them hidden
        }

        // Destroy all physical entities from the previous map iteration
        for (auto* bag : m_p1->GetGoldBags()) 
        { 
            if (bag) bag->MarkForDestroy(); 
        }
        for (auto* dia : m_p1->GetDiamonds()) 
        { 
            if (dia) dia->MarkForDestroy(); 
        }
        for (auto* enemy : m_p1->GetEnemies()) 
        { 
            if (enemy) enemy->MarkForDestroy(); 
        }
        for (auto* dirt : m_visualDirt) 
        { 
            if (dirt) dirt->MarkForDestroy(); 
        }
        for (auto* entity : m_miscEntities) 
        { 
            if (entity) entity->MarkForDestroy(); 
        }

        m_miscEntities.clear();
        m_p1->SetGoldBags({});
        m_p1->SetDiamonds({});
        m_p1->SetEnemies({});
        if (m_p2)
        {
            m_p2->SetGoldBags({});
            m_p2->SetDiamonds({});
            m_p2->SetEnemies({});
        }
        m_visualDirt.clear();

        // Reset and re-initialise the grid data structures
        m_cherrySpawnX = 0.0f;
        m_cherrySpawnY = 0.0f;
        m_cherrySpawned = false;

        m_p1->SetTotalEnemiesForLevel(0);
        if (m_p2) m_p2->SetTotalEnemiesForLevel(0);

        m_bonusMapActive = false;
        m_bonusFlickerPhase = false;

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
            // Swap Player 2's visual component to use the enemy Nob texture
            if (auto p2Render = m_p2->GetOwner()->GetComponent<RenderComponent>())
            {
                p2Render->SetTexture("PNG/Enemy/VNOB1.png");
            }
        }

        // 2. Safely toggle HUD visibility via coordinates (Never Destroy!)
        // Doing this avoids dangling pointers and reallocation overhead on a scene reset
        if (mode == GameMode::SinglePlayer)
        {
            if (m_pScoreUI1) m_pScoreUI1->SetLocalPosition(500, 13);
            if (m_p2Label) m_p2Label->SetLocalPosition(-1000, -1000); // Hide P2 Label
            if (m_p2Lives) m_p2Lives->SetLocalPosition(-1000, -1000); // Hide P2 Lives
        }
        else if (mode == GameMode::Versus)
        {
            if (m_pScoreUI1) m_pScoreUI1->SetLocalPosition(-1000, -1000); // Hide Score
            if (m_p2Label) m_p2Label->SetLocalPosition(905, 15); // Restore P2 Label
            if (m_p2Lives) m_p2Lives->SetLocalPosition(850, 15); // Restore P2 Lives
            if (m_p2LivesUI) m_p2LivesUI->SetTexture("PNG/Enemy/VNOB1.png");
        }
        else // CoOp
        {
            if (m_pScoreUI1) m_pScoreUI1->SetLocalPosition(500, 13); // Restore Score
            if (m_p2Label) m_p2Label->SetLocalPosition(905, 15); // Restore P2 Label
            if (m_p2Lives) m_p2Lives->SetLocalPosition(850, 15); // Restore P2 Lives
            if (m_p2LivesUI) m_p2LivesUI->SetTexture("PNG/Digger/VRDIG1X.png");
        }

        // Parse tile layout based on the string arrays stored in memory
        const auto layout = LevelManager::GetInstance().GetLevelLayout(levelIndex);
        constexpr float tileWidth = 40.0f;
        constexpr float startY = 52.0f;

        // Alternate the color palette of the dirt based on the level count
        const int textureNumber = (levelIndex % 3) + 1;
        const std::string normalTexture = "PNG/Map/VBACK" + std::to_string(textureNumber) + ".png";

        std::vector<GameObject*> newBags;
        std::vector<GameObject*> newDiamonds;
        std::vector<GameObject*> newEnemies;

        // Pass 1 - Construct the static background dirt tiles and empty tunnel hole objects
        for (int row = 0; row < static_cast<int>(layout.size()); ++row)
        {
            for (int col = 0; col < static_cast<int>(layout[row].size()); ++col)
            {
                const float bx = static_cast<float>(col) * tileWidth;
                const float by = startY + (static_cast<float>(row) * tileWidth);

                if (layout[row][col] != ' ')
                {
                    LevelManager::GetInstance().AddDirtTile(col, row);

                    // Stack 5 horizontal strips visually to create a dense 40x40 block 
                    // This allows Digger to partially eat into tiles visually.
                    for (int strip = 0; strip < 5; ++strip)
                    {
                        auto tile = std::make_unique<GameObject>();
                        tile->AddComponent<RenderComponent>(normalTexture);
                        tile->SetLocalPosition(bx, by + (static_cast<float>(strip) * 8.0f));
                        tile->SetZIndex(1);
                        m_visualDirt.push_back(tile.get());
                        m_pScene->Add(std::move(tile));
                    }
                }

                // Register an invisible "hole" object that turns black when dug through
                auto holeObj = std::make_unique<GameObject>();
                holeObj->SetLocalPosition(bx, by);
                holeObj->SetZIndex(2);
                LevelManager::GetInstance().RegisterHoleObject(col, row, holeObj.get());
                m_pScene->Add(std::move(holeObj));
            }
        }

        // Pass 2 - Spawn dynamic gameplay entities and assign player start positions
        for (int row = 0; row < static_cast<int>(layout.size()); ++row)
        {
            for (int col = 0; col < static_cast<int>(layout[row].size()); ++col)
            {
                const char c = layout[row][col];
                const float bx = static_cast<float>(col) * tileWidth;
                const float by = startY + (static_cast<float>(row) * tileWidth);

                // Auto-clear dirt underneath any spawn point so entities don't get stuck in walls
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
                    // Cap the AI processing by ensuring a spawner limits concurrent enemies
                    constexpr int k_MaxTotal = 4;
                    constexpr int k_MaxConcurrent = 2;

                    auto spawner = std::make_unique<GameObject>();
                    auto* spawnerC = spawner->AddComponent<EnemySpawnerComponent>(m_p1, m_p2, k_MaxTotal, k_MaxConcurrent);

                    m_p1->SetTotalEnemiesForLevel(k_MaxTotal);
                    if (m_p2) m_p2->SetTotalEnemiesForLevel(k_MaxTotal);

                    spawnerC->AddObserver(this); // notified when 75% spawned → SpawnCherry
                    spawner->SetLocalPosition(bx, by);
                    m_miscEntities.push_back(spawner.get());
                    m_pScene->Add(std::move(spawner));
                }
                else if (c == 'B')
                {
                    // Cherry spawn point — record position, no GameObject yet
                    m_cherrySpawnX = bx;
                    m_cherrySpawnY = by;
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

                    // The bag needs to know who the players are to crush them!
                    bagComp->SetPlayers(m_p1->GetOwner(), m_p2 ? m_p2->GetOwner() : nullptr);

                    goldBag->SetZIndex(4);
                    newBags.push_back(goldBag.get());
                    m_pScene->Add(std::move(goldBag));
                }
            }
        }

        // Register new arrays back with the player components so they can calculate collisions
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
        cherry->SetLocalPosition(m_cherrySpawnX, m_cherrySpawnY);
        cherry->SetZIndex(6);
        m_miscEntities.push_back(cherry.get());
        m_pScene->Add(std::move(cherry));
    }

    void LevelTransitionManager::ApplyDirtBrightness(bool enable)
    {
        for (auto* tile : m_visualDirt)
        {
            if (!tile || tile->IsMarkedForDestroy()) continue;

            // Alters the blending additive of the texture rendering to simulate a glowing effect
            if (auto* render = tile->GetComponent<RenderComponent>())
            {
                render->SetAdditiveBoost(enable); // default boostAlpha=120 (~47% brighter)
            }
        }
    }
}
