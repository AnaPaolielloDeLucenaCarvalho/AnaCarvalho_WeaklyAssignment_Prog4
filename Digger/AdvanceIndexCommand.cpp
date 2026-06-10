#include "AdvanceIndexCommand.h"
#include "NameEntryComponent.h"

namespace dae
{

AdvanceIndexCommand::AdvanceIndexCommand(NameEntryComponent* pEntry)
    : m_pEntry(pEntry)
{
}

void AdvanceIndexCommand::Execute(float /*deltaTime*/)
{
    if (m_pEntry)
    {
        m_pEntry->AdvanceIndex();
    }
}

} // namespace dae
