#include "LevelTransitionManager.h"

#include "Scene.h"
#include "LevelManager.h"
#include "DiggerComponent.h"
#include "GoldBagComponent.h"
#include "EnemySpawnerComponent.h"
#include "RenderComponent.h"
#include "CherryComponent.h"
#include "ServiceLocator.h"
#include "DiggerSounds.h"

#include <SDL3/SDL.h>
#include <string>

namespace dae
{
    LevelTransitionManager::LevelTransitionManager(GameObject* owner, Scene* scene, DiggerComponent* p1, DiggerComponent* p2)
        : Component(owner)
        , m_pScene(scene)
        , m_p1(p1)
        , m_p2(p2)
    {
    }

    void LevelTransitionManager::Update(float deltaTime)
    {
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
        // Hard audio reset — kill every track from the previous level before loading anything new
        ServiceLocator::GetSoundSystem().StopSfx();
        ServiceLocator::GetSoundSystem().StopMusic();
        ServiceLocator::GetSoundSystem().PlayMusic(DiggerSounds::MUSIC, 0.5f, true);

        // Destroy old entities
        for (auto* bag : m_p1->GetGoldBags()) { if (bag) bag->MarkForDestroy(); }
        for (auto* dia : m_p1->GetDiamonds()) { if (dia) dia->MarkForDestroy(); }
        for (auto* dirt : m_VisualDirt) { if (dirt) dirt->MarkForDestroy(); }

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
                    m_p1->GetOwner()->SetLocalPosition(bx, by);
                }
                else if (c == 'S' && m_p2)
                {
                    m_p2->SetSpawnPos({ bx, by });
                    m_p2->GetOwner()->SetLocalPosition(bx, by);
                }
                else if (c == 'E')
                {
                    constexpr int k_MaxTotal = 4;
                    constexpr int k_MaxConcurrent = 2;

                    auto spawner = std::make_unique<GameObject>();
                    auto* spawnerC = spawner->AddComponent<EnemySpawnerComponent>(m_p1, k_MaxTotal, k_MaxConcurrent);

                    m_p1->SetTotalEnemiesForLevel(k_MaxTotal);
                    if (m_p2) m_p2->SetTotalEnemiesForLevel(k_MaxTotal);

                    spawnerC->AddObserver(this); // notified when 75% spawned → SpawnCherry
                    spawner->SetLocalPosition(bx, by);
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
