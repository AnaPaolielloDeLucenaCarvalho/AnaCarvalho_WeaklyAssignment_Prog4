#include "CycleLetterCommand.h"
#include "NameEntryComponent.h"

namespace dae
{

    CycleLetterCommand::CycleLetterCommand(NameEntryComponent* pEntry, int direction)
        : m_pEntry(pEntry)
        , m_Direction(direction)
    {
    }

    void CycleLetterCommand::Execute(float /*deltaTime*/)
    {
        if (m_pEntry)
        {
            m_pEntry->CycleLetter(m_Direction);
        }
    }

} // namespace dae
