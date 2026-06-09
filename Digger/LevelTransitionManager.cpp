#include "LevelTransitionManager.h"

#include "Scene.h"
#include "LevelManager.h"
#include "DiggerComponent.h"
#include "GoldBagComponent.h"
#include "EnemySpawnerComponent.h"
#include "RenderComponent.h"

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

    void LevelTransitionManager::Update(float /*deltaTime*/) {}

    void LevelTransitionManager::OnNotify(EventId eventId, int /*value*/)
    {
        if (eventId == make_sdbm_hash("LoadNextLevel"))
        {
            const uint64_t currentTime = SDL_GetTicks();
            if (currentTime - m_LastLoadTime < 1000) return; // 1-second cooldown
            m_LastLoadTime = currentTime;

            m_CurrentLevelIndex++;
            if (m_CurrentLevelIndex >= LevelManager::GetInstance().GetTotalLevels())
            {
                m_CurrentLevelIndex = 0;
            }
            LoadLevel(m_CurrentLevelIndex);
        }
    }

    void LevelTransitionManager::LoadLevel(int levelIndex)
    {
        // Destroy old entities
        for (auto* bag : m_p1->GetGoldBags()) { if (bag) bag->MarkForDestroy(); }
        for (auto* dia : m_p1->GetDiamonds()) { if (dia) dia->MarkForDestroy(); }
        for (auto* dirt : m_VisualDirt) { if (dirt) dirt->MarkForDestroy(); }

        m_p1->SetGoldBags({});
        m_p1->SetDiamonds({});
        if (m_p2)
        {
            m_p2->SetGoldBags({});
            m_p2->SetDiamonds({});
        }
        m_VisualDirt.clear();

        // Reset and re-initialise the grid
        LevelManager::GetInstance().ClearLevel();
        m_pScene->RequestLevelCleanup(); // Scene cleans up at end of frame
        LevelManager::GetInstance().InitLevel(14, 26);

        // Parse tile layout
        const auto layout = LevelManager::GetInstance().GetLevelLayout(levelIndex);
        constexpr float tileWidth = 40.0f;
        constexpr float startY = 52.0f;

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

                    // Cycle through three background textures based on level index
                    const int textureNumber = (levelIndex % 3) + 1;
                    const std::string texturePath = "PNG/Map/VBACK" + std::to_string(textureNumber) + ".png";

                    for (int strip = 0; strip < 5; ++strip)
                    {
                        auto tile = std::make_unique<GameObject>();
                        tile->AddComponent<RenderComponent>(texturePath);
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

                // Passable tiles are dug out
                if (c == ' ' || c == 'P' || c == 'S' || c == 'E')
                {
                    LevelManager::GetInstance().Dig(bx, by);
                }

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
                    // Enemy spawner placed exactly at the 'E' tile position
                    auto spawner = std::make_unique<GameObject>();
                    spawner->AddComponent<EnemySpawnerComponent>(m_p1, 4, 2);
                    spawner->SetLocalPosition(bx, by);
                    m_pScene->Add(std::move(spawner));
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
}
