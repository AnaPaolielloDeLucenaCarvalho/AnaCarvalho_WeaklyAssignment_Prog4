#ifndef LEVELMANAGER_H
#define LEVELMANAGER_H

#include <string>
#include <vector>
#include "Singleton.h"

namespace dae
{
    enum class GameMode { SinglePlayer, CoOp, Versus };

    class GameObject;

    class LevelManager : public Singleton<LevelManager>
    {
    public:
        void InitLevel(int rows, int cols);

        void LoadAllLevelsFromFile(const std::string& filePath);
        std::vector<std::string> GetLevelLayout(int levelIndex) const;
        int GetTotalLevels() const { return static_cast<int>(m_AllLevelLayouts.size()); }

        void AddDirtTile(int col, int row);
        void RegisterHoleObject(int col, int row, GameObject* pObj);

        bool IsDirtAt(float x, float y) const;
        bool IsDug(float x, float y) const;
        void Dig(float x, float y);

        float GetGridSize() const { return m_GridSize; }
        float GetOffsetY() const { return m_OffsetY; }

        void ClearLevel();

        void SetGameMode(GameMode mode) { m_CurrentGameMode = mode; }
        GameMode GetGameMode() const { return m_CurrentGameMode; }

        void SetWinnerText(const std::string& text) { m_WinnerText = text; }
        std::string GetWinnerText() const { return m_WinnerText; }

        void SetNeedsGameReset(bool needsReset) { m_NeedsGameReset = needsReset; }
        bool NeedsGameReset() const { return m_NeedsGameReset; }

    private:
        friend class Singleton<LevelManager>;
        LevelManager() = default;

        float m_GridSize{ 40.0f };
        float m_OffsetY{ 52.0f };

        std::vector<std::vector<bool>> m_DirtGrid;
        std::vector<std::vector<GameObject*>> m_HoleObjects;

        std::vector<std::vector<std::string>> m_AllLevelLayouts;
        
        GameMode m_CurrentGameMode{ GameMode::SinglePlayer };
        std::string m_WinnerText{""};
        bool m_NeedsGameReset{false};
    };
}

#endif