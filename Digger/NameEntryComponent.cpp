#include "NameEntryComponent.h"

#include "TextComponent.h"
#include "HighScoreManager.h"
#include "SceneManager.h"
#include "Scene.h"

#include <string>

namespace dae
{


    NameEntryComponent::NameEntryComponent(GameObject* owner, TextComponent* pText, HighScoreManager* pManager, Scene* pGameScene)
        : Component(owner)
        , m_pText (pText)
        , m_pManager (pManager)
        , m_pGameScene(pGameScene)
        , m_Initials { 'A', 'A', 'A' }
        , m_CurrentIndex(0)
        , m_InputCooldown(0.3f)
    {
        RefreshDisplay();
    }

    void NameEntryComponent::Update(float deltaTime)
    {
        if (m_InputCooldown > 0.0f)
        {
            m_InputCooldown -= deltaTime;
        }
    }

    void NameEntryComponent::CycleLetter(int direction)
    {
        if (m_InputCooldown > 0.0f) return;
        m_InputCooldown = 0.15f;

        // Wrap A-Z in both directions.
        int current = m_Initials[m_CurrentIndex] - 'A'; // 0-25
        current = (current + direction + 26) % 26;
        m_Initials[m_CurrentIndex] = static_cast<char>('A' + current);

        RefreshDisplay();
    }

    void NameEntryComponent::AdvanceIndex()
    {
        if (m_InputCooldown > 0.0f) return;
        m_InputCooldown = 0.15f;

        m_CurrentIndex = (m_CurrentIndex + 1) % 3;
        RefreshDisplay();
    }

    void NameEntryComponent::ConfirmName()
    {
        if (m_InputCooldown > 0.0f) return;
        m_InputCooldown = 0.15f;

        std::string initials;
        initials += m_Initials[0];
        initials += m_Initials[1];
        initials += m_Initials[2];

        m_pManager->SetSessionName(initials);

        SceneManager::GetInstance().SetActiveScene(m_pGameScene);
    }

    void NameEntryComponent::RefreshDisplay()
    {
        if (!m_pText) return;

        std::string display;
        for (int i = 0; i < 3; ++i)
        {
            if (i == m_CurrentIndex)
            {
                display += '[';
                display += m_Initials[i];
                display += ']';
            }
            else
            {
                display += ' ';
                display += m_Initials[i];
                display += ' ';
            }

            if (i < 2) display += ' ';
        }

        m_pText->SetText(display);
    }

} // namespace dae
