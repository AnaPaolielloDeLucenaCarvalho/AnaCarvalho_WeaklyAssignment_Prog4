#include "NameEntryCommands.h"
#include "NameEntryComponent.h"
#include "SceneManager.h"

// Implementations of the commands related to name entry (advancing index, confirming name, cycling through letters)
// Each command checks if the active scene matches the target scene before executing

namespace dae
{
// Command to advance the index of the name entry (moving between initials)
    AdvanceIndexCommand::AdvanceIndexCommand(NameEntryComponent* pEntry, int direction, Scene* targetScene)
        : m_pEntry(pEntry)
        , m_direction(direction)
        , m_pTargetScene(targetScene)
    {
    }

    void AdvanceIndexCommand::Execute(float /*deltaTime*/)
    {
        // Scene Guard - We only execute this command if the player is looking at the Name Entry scene.
        // Why - Prevents player from altering their name while mashing buttons in the Main Menu.
        if (dae::SceneManager::GetInstance().GetActiveScene() != m_pTargetScene) return;

        // Null check to ensure the receiver still exists in memory before calling its method
        if (m_pEntry)
        {
            m_pEntry->AdvanceIndex(m_direction);
        }
    }

// Command to confirm the entered name
    ConfirmNameCommand::ConfirmNameCommand(NameEntryComponent* pEntry, Scene* targetScene)
        : m_pEntry(pEntry)
        , m_pTargetScene(targetScene)
    {
    }

    void ConfirmNameCommand::Execute(float /*deltaTime*/)
    {
        // Scene Guard - Only execute if looking at the exact scene this command is meant for
        if (dae::SceneManager::GetInstance().GetActiveScene() != m_pTargetScene) return;

        if (m_pEntry)
        {
            m_pEntry->ConfirmName();
        }
    }

// Command to cycle through the letters for the current initial
    CycleLetterCommand::CycleLetterCommand(NameEntryComponent* pEntry, int direction, Scene* targetScene)
        : m_pEntry(pEntry)
        , m_direction(direction)
        , m_pTargetScene(targetScene)
    {
    }

    void CycleLetterCommand::Execute(float /*deltaTime*/)
    {
        // Scene Guard - Only execute if looking at the exact scene this command is meant for
        if (dae::SceneManager::GetInstance().GetActiveScene() != m_pTargetScene) return;

        if (m_pEntry)
        {
            m_pEntry->CycleLetter(m_direction);
        }
    }
}
