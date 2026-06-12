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
        // Pre-allocate 2D boolean grid structures instead of using heavy physics colliders for map tiles
        m_dirtGrid.assign(rows, std::vector<bool>(cols, false));
        m_holeObjects.assign(rows, std::vector<GameObject*>(cols, nullptr));
    }

    void LevelManager::AddDirtTile(int col, int row)
    {
        if (row >= 0 && row < static_cast<int>(m_dirtGrid.size()) && col >= 0 && col < static_cast<int>(m_dirtGrid[0].size()))
        {
            m_dirtGrid[row][col] = true;
        }
    }

    void LevelManager::RegisterHoleObject(int col, int row, GameObject* pObj)
    {
        if (row >= 0 && row < static_cast<int>(m_holeObjects.size()) && col >= 0 && col < static_cast<int>(m_holeObjects[0].size()))
        {
            m_holeObjects[row][col] = pObj;
        }
    }

    bool LevelManager::IsDirtAt(float x, float y) const
    {
        // Convert physical world coordinate boundaries into discrete grid array index locations
        int col = static_cast<int>(std::round(x / m_gridSize));
        int row = static_cast<int>(std::round((y - m_offsetY) / m_gridSize));

        // Array Bounds Guard to prevent hard crashes if AI tries to pathfind completely off-screen
        if (row < 0 || row >= static_cast<int>(m_dirtGrid.size()) || col < 0 || col >= static_cast<int>(m_dirtGrid[0].size())) return false;

        return m_dirtGrid[row][col];
    }

    bool LevelManager::IsDug(float x, float y) const
    {
        int col = static_cast<int>(std::round(x / m_gridSize));
        int row = static_cast<int>(std::round((y - m_offsetY) / m_gridSize));

        if (row < 0 || row >= static_cast<int>(m_dirtGrid.size()) || col < 0 || col >= static_cast<int>(m_dirtGrid[0].size())) return true;

        return !m_dirtGrid[row][col];
    }

    void LevelManager::Dig(float x, float y)
    {
        int col = static_cast<int>(std::round(x / m_gridSize));
        int row = static_cast<int>(std::round((y - m_offsetY) / m_gridSize));

        if (row >= 0 && row < static_cast<int>(m_dirtGrid.size()) && col >= 0 && col < static_cast<int>(m_dirtGrid[0].size()))
        {
            if (m_dirtGrid[row][col])
            {
                // Flips the boolean to mark the path as traversable for AI pathfinding
                m_dirtGrid[row][col] = false;

                // Dynamically append a black UI panel component over the visually static dirt to create a tunnel illusion
                if (m_holeObjects[row][col])
                {
                    m_holeObjects[row][col]->AddComponent<UIPanelComponent>(m_gridSize, m_gridSize, SDL_Color{ 0, 0, 0, 255 });
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

        m_allLevelLayouts.clear();
        std::vector<std::string> currentLayout;
        std::string line;

        // File Parsing Strategy:
        // We iterate through the raw text file line-by-line, utilizing the "[LEVEL]" tags 
        // as delimiters to segment the map layouts into memory buffers.
        while (std::getline(file, line))
        {
            // Sanitize carriage returns that can accidentally corrupt text parsing on different OS environments
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
                // Flush the completed layout block into the global array map before advancing to the next map text block
                if (!currentLayout.empty())
                {
                    m_allLevelLayouts.push_back(currentLayout);
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
            m_allLevelLayouts.push_back(currentLayout);
        }

        std::cout << "LevelManager: Successfully loaded " << m_allLevelLayouts.size() << " levels!\n";
    }

    std::vector<std::string> LevelManager::GetLevelLayout(int levelIndex) const
    {
        if (levelIndex >= 0 && levelIndex < static_cast<int>(m_allLevelLayouts.size()))
        {
            return m_allLevelLayouts[levelIndex];
        }
        return {};
    }

    void LevelManager::ClearLevel()
    {
        // Safe garbage collection: Flag the items for Engine deletion rather than forcefully executing raw deletes
        for (const auto& row : m_holeObjects)
        {
            for (auto* pObj : row)
            {
                if (pObj) pObj->MarkForDestroy();
            }
        }
        m_holeObjects.clear();
        m_dirtGrid.clear();
    }
}