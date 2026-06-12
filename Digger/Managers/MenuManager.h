#ifndef MENUMANAGER_H
#define MENUMANAGER_H

#include "Component.h"
#include "TextComponent.h"
#include <vector>

// DESIGN PATTERN - Component Pattern
// Built the MenuManager as a Component attached to GameObject. This keeps the Scene clean and purely data-driven. Instead of hardcoding menu navigation directly into the Main method or Scene loop, this component acts as the "brain" for the Main Menu

namespace dae
{
    // ARCHITECTURAL DECISION - Forward Declarations
    // By forward-declaring these classes instead of using #include "Scene.h", I prevent the compiler from rebuilding every single file in the game if I make a tiny change to the Scene class. It significantly speeds up compile times!
    class Scene;
    class HighScoreManager;

    class MenuManager final : public Component
    {
    public:
        MenuManager(GameObject* owner, HighScoreManager* pMgr, Scene* pScoreScene, Scene* pGameScene, const std::vector<TextComponent*>& options, const std::vector<TextComponent*>& scoreTexts, const std::vector<TextComponent*>& nameTexts);

        void Render() const override {}
        void Update(float deltaTime) override;

        // Exposed public methods so the external UICommands can control the menu without the MenuManager needing to poll hardware inputs directly.
        void NavigateUp();
        void NavigateDown();
        void Select();

    private:
        HighScoreManager* m_pMgr;
        Scene* m_pScoreScene;
        Scene* m_pGameScene;
        std::vector<TextComponent*> m_options;
        std::vector<TextComponent*> m_scoreTexts;
        std::vector<TextComponent*> m_nameTexts;
        int m_selectedIndex{ 0 };
        float m_refreshTimer{ 0.0f };
    };
}

#endif // MENUMANAGER_H