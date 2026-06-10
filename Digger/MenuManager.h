#ifndef MENUMANAGER_H
#define MENUMANAGER_H

#include "Component.h"

namespace dae
{
    class Scene; // forward declaration — full type only needed in .cpp
    class HighScoreManager; // forward declaration — full type only needed in .cpp

    class MenuManager final : public Component
    {
    public:
        MenuManager(GameObject* owner, HighScoreManager* pMgr, Scene* pScoreScene, Scene* pGameScene);

        void Update(float deltaTime) override;

    private:
        HighScoreManager* m_pMgr;
        Scene* m_pScoreScene;
        Scene* m_pGameScene;
    };
}

#endif // MENUMANAGER_H