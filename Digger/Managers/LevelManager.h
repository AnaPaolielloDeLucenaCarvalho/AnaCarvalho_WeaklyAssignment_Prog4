#ifndef LEVELMANAGER_H
#define LEVELMANAGER_H

#include <string>
#include <vector>
#include "Singleton.h"

// DESIGN PATTERN: Singleton
// The LevelManager is a globally accessible Singleton because virtually every sub-system  (AI Pathfinding, Player Collisions, UI Renderers) requires instantaneous access to the underlying collision grid arrays without passing pointers endlessly.

namespace dae
{
    enum class GameMode { SinglePlayer, CoOp, Versus };

    class GameObject;

    class LevelManager final : public Singleton<LevelManager>
    {
    public:
        void InitLevel(int rows, int cols);

        void LoadAllLevelsFromFile(const std::string& filePath);
        std::vector<std::string> GetLevelLayout(int levelIndex) const;
        int GetTotalLevels() const { return static_cast<int>(m_allLevelLayouts.size()); }

        void AddDirtTile(int col, int row);
        void RegisterHoleObject(int col, int row, GameObject* pObj);

        bool IsDirtAt(float x, float y) const;
        bool IsDug(float x, float y) const;
        void Dig(float x, float y);

        float GetGridSize() const { return m_gridSize; }
        float GetOffsetY() const { return m_offsetY; }

        void ClearLevel();

        void SetGameMode(GameMode mode) { m_currentGameMode = mode; }
        GameMode GetGameMode() const { return m_currentGameMode; }

        void SetWinnerText(const std::string& text) { m_winnerText = text; }
        std::string GetWinnerText() const { return m_winnerText; }

        void SetNeedsGameReset(bool needsReset) { m_needsGameReset = needsReset; }
        bool NeedsGameReset() const { return m_needsGameReset; }

    private:
        friend class Singleton<LevelManager>;
        LevelManager() = default;

        float m_gridSize{ 40.0f };
        float m_offsetY{ 52.0f };

        std::vector<std::vector<bool>> m_dirtGrid;
        std::vector<std::vector<GameObject*>> m_holeObjects;

        std::vector<std::vector<std::string>> m_allLevelLayouts;

        GameMode m_currentGameMode{ GameMode::SinglePlayer };
        std::string m_winnerText{ "" };
        bool m_needsGameReset{ false };
    };
}

#endif