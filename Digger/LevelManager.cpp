#include "LevelManager.h"
#include "GameObject.h"
#include "UIObservers.h"
#include <cmath>

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

        if (row < 0 || row >= static_cast<int>(m_DirtGrid.size()) || col < 0 || col >= static_cast<int>(m_DirtGrid[0].size()))
            return false;

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
}