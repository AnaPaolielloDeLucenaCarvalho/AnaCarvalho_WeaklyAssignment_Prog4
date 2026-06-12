#include "NameEntryCommands.h"
#include "NameEntryComponent.h"
#include "SceneManager.h"

namespace dae
{
    AdvanceIndexCommand::AdvanceIndexCommand(NameEntryComponent* pEntry, int direction, Scene* targetScene)
        : m_pEntry(pEntry)
        , m_Direction(direction)
        , m_pTargetScene(targetScene)
    {
    }

    void AdvanceIndexCommand::Execute(float /*deltaTime*/)
    {
        if (dae::SceneManager::GetInstance().GetActiveScene() != m_pTargetScene) return;

        if (m_pEntry)
        {
            m_pEntry->AdvanceIndex(m_Direction);
        }
    }

    ConfirmNameCommand::ConfirmNameCommand(NameEntryComponent* pEntry, Scene* targetScene)
        : m_pEntry(pEntry)
        , m_pTargetScene(targetScene)
    {
    }

    void ConfirmNameCommand::Execute(float /*deltaTime*/)
    {
        if (dae::SceneManager::GetInstance().GetActiveScene() != m_pTargetScene) return;

        if (m_pEntry)
        {
            m_pEntry->ConfirmName();
        }
    }

    CycleLetterCommand::CycleLetterCommand(NameEntryComponent* pEntry, int direction, Scene* targetScene)
        : m_pEntry(pEntry)
        , m_Direction(direction)
        , m_pTargetScene(targetScene)
    {
    }

    void CycleLetterCommand::Execute(float /*deltaTime*/)
    {
        if (dae::SceneManager::GetInstance().GetActiveScene() != m_pTargetScene) return;

        if (m_pEntry)
        {
            m_pEntry->CycleLetter(m_Direction);
        }
    }
}
