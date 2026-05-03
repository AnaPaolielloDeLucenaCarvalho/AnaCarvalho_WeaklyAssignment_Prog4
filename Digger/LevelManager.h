#ifndef LEVELMANAGER_H
#define LEVELMANAGER_H

#include <string>
#include <vector>
#include "Singleton.h"

namespace dae
{
    class GameObject;

    class LevelManager : public Singleton<LevelManager>
    {
    public:
        void InitLevel(int rows, int cols);
        void AddDirtTile(int col, int row);
        void RegisterHoleObject(int col, int row, GameObject* pObj);

        bool IsDirtAt(float x, float y) const;
        void Dig(float x, float y);

        float GetGridSize() const { return m_GridSize; }
        float GetOffsetY() const { return m_OffsetY; }

    private:
        friend class Singleton<LevelManager>;
        LevelManager() = default;

        float m_GridSize{ 40.0f };
        float m_OffsetY{ 52.0f };

        std::vector<std::vector<bool>> m_DirtGrid;
        std::vector<std::vector<GameObject*>> m_HoleObjects;
    };
}
#endif