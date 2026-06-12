#include "PlayerCommands.h"
#include "DiggerComponent.h"

namespace dae
{
// MoveCommand - Makes Digger move in a specified direction. 
    MoveCommand::MoveCommand(DiggerComponent* pDigger, const glm::vec2& direction)
        : m_pDigger(pDigger)
        , m_Direction(direction)
    {
    }

    void MoveCommand::Execute(float /*deltaTime*/)
    {
        // Safe pointer check before delegating the action to the Receiver (DiggerComponent)
        if (m_pDigger)
        {
            m_pDigger->SetDesiredDirection(m_Direction);
        }
    }

// ShootCommand - Makes Digger shoot a fireball in the last faced direction, if possible (not on cooldown).
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
