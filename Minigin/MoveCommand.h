#ifndef MOVECOMMAND_H
#define MOVECOMMAND_H

#include "Command.h"
#include "DiggerComponent.h"
#include <glm/glm.hpp>

namespace dae
{
    class MoveCommand : public Command
    {
    public:
        MoveCommand(DiggerComponent* pDigger, const glm::vec2& direction)
            : m_pDigger(pDigger), m_Direction(direction) {
        }

        void Execute(float /*deltaTime*/) override
        {
            if (m_pDigger)
            {
                m_pDigger->SetDesiredDirection(m_Direction);
            }
        }
    private:
        DiggerComponent* m_pDigger;
        glm::vec2 m_Direction;
    };
}
#endif