#include "ShootCommand.h"
#include "DiggerComponent.h"

namespace dae
{
    ShootCommand::ShootCommand(DiggerComponent* pDigger)
        : m_pDigger(pDigger)
    {
    }

    void ShootCommand::Execute(float /*deltaTime*/)
    {
        if (m_pDigger)
        {
            m_pDigger->Shoot();
        }
    }
}
