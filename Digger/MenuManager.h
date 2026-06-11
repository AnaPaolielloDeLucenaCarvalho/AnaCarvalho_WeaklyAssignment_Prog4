#ifndef MENUMANAGER_H
#define MENUMANAGER_H

#include "Component.h"
#include "TextComponent.h"
#include <vector>

namespace dae
{
    class Scene; // forward declaration — full type only needed in .cpp
    class HighScoreManager; // forward declaration — full type only needed in .cpp

    class MenuManager final : public Component
    {
    public:
        MenuManager(GameObject* owner, HighScoreManager* pMgr, Scene* pScoreScene, Scene* pGameScene, const std::vector<TextComponent*>& options);

        void Update(float deltaTime) override;

    private:
        HighScoreManager* m_pMgr;
        Scene* m_pScoreScene;
        Scene* m_pGameScene;
        std::vector<TextComponent*> m_Options;
        int m_SelectedIndex{ 0 };
        float m_InputCooldown{ 0.0f };
    };
}

#endif // MENUMANAGER_H