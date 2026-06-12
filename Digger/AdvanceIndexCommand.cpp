#include "AdvanceIndexCommand.h"
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

} // namespace dae
