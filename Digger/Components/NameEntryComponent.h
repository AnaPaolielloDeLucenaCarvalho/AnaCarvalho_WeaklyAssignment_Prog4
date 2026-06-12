#ifndef NAME_ENTRY_COMPONENT_H
#define NAME_ENTRY_COMPONENT_H

#include "Component.h"
#include <string>

// DESIGN PATTERN - Component Pattern
// This encapsulates the arcade-style name entry logic (A-Z scrolling). By keeping it separate  from the Scene itself, the Scene just passes UI commands to this component, maintaining strict data separation.

namespace dae
{
    class TextComponent;
    class HighScoreManager;
    class Scene;

    class NameEntryComponent final : public Component
    {
    public:
        NameEntryComponent(GameObject* owner, TextComponent* pText, HighScoreManager* pManager, Scene* pGameScene);

        void Update(float deltaTime) override;

        void CycleLetter(int direction);
        void AdvanceIndex(int direction);
        void ConfirmName();

    private:
        void RefreshDisplay();

        TextComponent* m_pText;
        HighScoreManager* m_pManager;
        Scene* m_pGameScene;

        char m_Initials[3];
        int m_CurrentIndex;
        float m_InputCooldown{ 0.0f };
    };

}

#endif // NAME_ENTRY_COMPONENT_H