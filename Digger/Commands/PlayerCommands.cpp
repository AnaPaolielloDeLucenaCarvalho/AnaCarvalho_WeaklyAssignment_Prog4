#include "PlayerCommands.h"
#include "DiggerComponent.h"

namespace dae
{
    MoveCommand::MoveCommand(DiggerComponent* pDigger, const glm::vec2& direction)
        : m_pDigger(pDigger)
        , m_Direction(direction)
    {
    }

    void MoveCommand::Execute(float /*deltaTime*/)
    {
        if (m_pDigger)
        {
            m_pDigger->SetDesiredDirection(m_Direction);
        }
    }

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
