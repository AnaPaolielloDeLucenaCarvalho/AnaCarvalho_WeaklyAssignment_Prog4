#ifndef NAME_ENTRY_COMPONENT_H
#define NAME_ENTRY_COMPONENT_H

#include "Component.h"
#include <string>

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
        float m_InputCooldown{0.0f};
    };

}

#endif // NAME_ENTRY_COMPONENT_H
