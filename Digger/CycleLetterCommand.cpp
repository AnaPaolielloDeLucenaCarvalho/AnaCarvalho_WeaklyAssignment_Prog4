#include "CycleLetterCommand.h"
#include "NameEntryComponent.h"

#include "SceneManager.h"

namespace dae
{

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

} // namespace dae
