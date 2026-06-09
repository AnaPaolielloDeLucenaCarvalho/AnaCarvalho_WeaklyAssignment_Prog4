#include "SkipLevelCommand.h"

#include "DiggerComponent.h"
#include "Observer.h"

namespace dae
{
    SkipLevelCommand::SkipLevelCommand(DiggerComponent* pDigger)
        : m_pDigger(pDigger)
    {
    }

    void SkipLevelCommand::Execute(float /*deltaTime*/)
    {
        if (m_pDigger)
        {
            m_pDigger->GetSubject().Notify(make_sdbm_hash("LoadNextLevel"), 0);
        }
    }
}
