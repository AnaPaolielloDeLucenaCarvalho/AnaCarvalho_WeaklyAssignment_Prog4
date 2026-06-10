#include "LevelManager.h"
#include "GameObject.h"
#include "UIObservers.h"
#include <cmath>
#include <fstream>
#include <iostream>

namespace dae
{
    void LevelManager::InitLevel(int rows, int cols)
    {
        m_DirtGrid.assign(rows, std::vector<bool>(cols, false));
        m_HoleObjects.assign(rows, std::vector<GameObject*>(cols, nullptr));
    }

    void LevelManager::AddDirtTile(int col, int row)
    {
        if (row >= 0 && row < static_cast<int>(m_DirtGrid.size()) && col >= 0 && col < static_cast<int>(m_DirtGrid[0].size()))
        {
            m_DirtGrid[row][col] = true;
        }
    }

    void LevelManager::RegisterHoleObject(int col, int row, GameObject* pObj)
    {
        if (row >= 0 && row < static_cast<int>(m_HoleObjects.size()) && col >= 0 && col < static_cast<int>(m_HoleObjects[0].size()))
        {
            m_HoleObjects[row][col] = pObj;
        }
    }

    bool LevelManager::IsDirtAt(float x, float y) const
    {
        int col = static_cast<int>(std::round(x / m_GridSize));
        int row = static_cast<int>(std::round((y - m_OffsetY) / m_GridSize));

        if (row < 0 || row >= static_cast<int>(m_DirtGrid.size()) || col < 0 || col >= static_cast<int>(m_DirtGrid[0].size())) return false;

        return m_DirtGrid[row][col];
    }

    void LevelManager::Dig(float x, float y)
    {
        int col = static_cast<int>(std::round(x / m_GridSize));
        int row = static_cast<int>(std::round((y - m_OffsetY) / m_GridSize));

        if (row >= 0 && row < static_cast<int>(m_DirtGrid.size()) && col >= 0 && col < static_cast<int>(m_DirtGrid[0].size()))
        {
            if (m_DirtGrid[row][col])
            {
                m_DirtGrid[row][col] = false;

                if (m_HoleObjects[row][col])
                {
                    m_HoleObjects[row][col]->AddComponent<UIPanelComponent>(m_GridSize, m_GridSize, SDL_Color{ 0, 0, 0, 255 });
                }
            }
        }
    }

    void LevelManager::LoadAllLevelsFromFile(const std::string& filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            std::cerr << "LevelManager ERROR: Could not open level file: " << filePath << "\n";
            return;
        }

        m_AllLevelLayouts.clear();
        std::vector<std::string> currentLayout;
        std::string line;

        while (std::getline(file, line))
        {
            if (!line.empty() && line.back() == '\r') 
            {
                line.pop_back();
            }

            // 'X' = Dirt
            // ' ' = Empty
            // 'P' = Player 1
            // 'S' = Player 2
			// 'E' = Enemy
            // 'D' = Diamond
            // 'C' = Coin Bag
			// 'B' = Bonus cherry spawn point

			if (line == "[LEVEL]") // In the .txt file i separated the 3 levels using [LEVEL]
            {
                if (!currentLayout.empty())
                {
                    m_AllLevelLayouts.push_back(currentLayout);
                    currentLayout.clear();
                }
            }
            else if (!line.empty())
            {
                currentLayout.push_back(line);
            }
        }

        if (!currentLayout.empty())
        {
            m_AllLevelLayouts.push_back(currentLayout);
        }

        std::cout << "LevelManager: Successfully loaded " << m_AllLevelLayouts.size() << " levels!\n";
    }

    std::vector<std::string> LevelManager::GetLevelLayout(int levelIndex) const
    {
        if (levelIndex >= 0 && levelIndex < static_cast<int>(m_AllLevelLayouts.size()))
        {
            return m_AllLevelLayouts[levelIndex];
        }
        return {};
    }

    void LevelManager::ClearLevel()
    {
        for (auto& row : m_HoleObjects)
        {
            for (auto* pObj : row)
            {
                if (pObj) pObj->MarkForDestroy();
            }
        }
        m_HoleObjects.clear();
        m_DirtGrid.clear();
    }
}