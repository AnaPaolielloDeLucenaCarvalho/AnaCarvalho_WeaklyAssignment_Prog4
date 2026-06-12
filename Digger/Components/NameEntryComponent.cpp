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
        , m_initials{ 'A', 'A', 'A' }
        , m_currentIndex(0)
        , m_inputCooldown(0.3f)
    {
        RefreshDisplay();
    }

    void NameEntryComponent::Update(float deltaTime)
    {
        // Basic debounce timer to prevent ultra-fast inputs from scrolling the letters too quickly
        if (m_inputCooldown > 0.0f)
        {
            m_inputCooldown -= deltaTime;
        }
    }

    void NameEntryComponent::CycleLetter(int direction)
    {
        if (m_inputCooldown > 0.0f) return;
        m_inputCooldown = 0.15f;

        // Wrap A-Z in both directions using modulo math. 
        // We add 26 before the modulo to handle negative wrap-arounds safely.
        int current = m_initials[m_currentIndex] - 'A'; // Normalize ascii char to 0-25
        current = (current + direction + 26) % 26;
        m_initials[m_currentIndex] = static_cast<char>('A' + current); // Convert back to ASCII char

        RefreshDisplay();
    }

    void NameEntryComponent::AdvanceIndex(int direction)
    {
        if (m_inputCooldown > 0.0f) return;
        m_inputCooldown = 0.15f;

        // Move the cursor left or right across the 3 available initial slots, wrapping around
        m_currentIndex = (m_currentIndex + direction) % 3;
        if (m_currentIndex < 0) m_currentIndex += 3;
        RefreshDisplay();
    }

    void NameEntryComponent::ConfirmName()
    {
        if (m_inputCooldown > 0.0f) return;
        m_inputCooldown = 0.15f;

        // Build the final 3-character string from the array
        std::string initials;
        initials += m_initials[0];
        initials += m_initials[1];
        initials += m_initials[2];

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
            if (i == m_currentIndex)
            {
                display += '[';
                display += m_initials[i];
                display += ']';
            }
            else
            {
                display += ' ';
                display += m_initials[i];
                display += ' ';
            }

            if (i < 2) display += ' '; // Add spacing between the letters
        }

        m_pText->SetText(display);
    }

} // namespace dae