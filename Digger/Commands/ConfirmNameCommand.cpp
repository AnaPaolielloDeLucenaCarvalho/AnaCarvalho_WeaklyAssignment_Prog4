#include "ConfirmNameCommand.h"
#include "NameEntryComponent.h"

#include "SceneManager.h"

namespace dae
{

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

} // namespace dae
