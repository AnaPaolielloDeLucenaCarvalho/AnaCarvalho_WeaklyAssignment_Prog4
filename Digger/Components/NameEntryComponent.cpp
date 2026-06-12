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
        , m_pText(pText)
        , m_pManager(pManager)
        , m_pGameScene(pGameScene)
        , m_Initials{ 'A', 'A', 'A' }
        , m_CurrentIndex(0)
        , m_InputCooldown(0.3f)
    {
        RefreshDisplay();
    }

    void NameEntryComponent::Update(float deltaTime)
    {
        // Basic debounce timer to prevent ultra-fast inputs from scrolling the letters too quickly
        if (m_InputCooldown > 0.0f)
        {
            m_InputCooldown -= deltaTime;
        }
    }

    void NameEntryComponent::CycleLetter(int direction)
    {
        if (m_InputCooldown > 0.0f) return;
        m_InputCooldown = 0.15f;

        // Wrap A-Z in both directions using modulo math. 
        // We add 26 before the modulo to handle negative wrap-arounds safely.
        int current = m_Initials[m_CurrentIndex] - 'A'; // Normalize ascii char to 0-25
        current = (current + direction + 26) % 26;
        m_Initials[m_CurrentIndex] = static_cast<char>('A' + current); // Convert back to ASCII char

        RefreshDisplay();
    }

    void NameEntryComponent::AdvanceIndex(int direction)
    {
        if (m_InputCooldown > 0.0f) return;
        m_InputCooldown = 0.15f;

        // Move the cursor left or right across the 3 available initial slots, wrapping around
        m_CurrentIndex = (m_CurrentIndex + direction) % 3;
        if (m_CurrentIndex < 0) m_CurrentIndex += 3;
        RefreshDisplay();
    }

    void NameEntryComponent::ConfirmName()
    {
        if (m_InputCooldown > 0.0f) return;
        m_InputCooldown = 0.15f;

        // Build the final 3-character string from the array
        std::string initials;
        initials += m_Initials[0];
        initials += m_Initials[1];
        initials += m_Initials[2];

        // Save the active session name into the global HighScore system
        m_pManager->SetSessionName(initials);

        // Transition from Name Entry into the actual gameplay loop
        SceneManager::GetInstance().SetActiveScene(m_pGameScene);
    }

    void NameEntryComponent::RefreshDisplay()
    {
        if (!m_pText) return;

        std::string display;

        // Loop through the initials and wrap the currently selected one in brackets to create a visual cursor
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

            if (i < 2) display += ' '; // Add spacing between the letters
        }

        m_pText->SetText(display);
    }

} // namespace dae