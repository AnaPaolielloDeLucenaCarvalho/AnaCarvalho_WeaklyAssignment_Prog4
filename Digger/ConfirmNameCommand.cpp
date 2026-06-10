#include "ConfirmNameCommand.h"
#include "NameEntryComponent.h"

namespace dae
{

ConfirmNameCommand::ConfirmNameCommand(NameEntryComponent* pEntry)
    : m_pEntry(pEntry)
{
}

void ConfirmNameCommand::Execute(float /*deltaTime*/)
{
    if (m_pEntry)
    {
        m_pEntry->ConfirmName();
    }
}

} // namespace dae
