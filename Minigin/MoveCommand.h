#ifndef MOVE_COMMAND_H
#define MOVE_COMMAND_H

#include "Command.h"
#include "GameObject.h"
#include <glm/glm.hpp>

namespace dae
{
    class MoveCommand : public Command
    {
    public:
        MoveCommand(GameObject* gameObject, const glm::vec2& direction, float speed)
            : m_pGameObject(gameObject)
            , m_Direction(direction)
            , m_Speed(speed)
        {
			// feedback - use more algorithm's when we can (normalize)
            if (glm::length(m_Direction) > 0)
            {
                m_Direction = glm::normalize(m_Direction);
            }
        }

        void Execute(float deltaTime) override
        {
            if (m_pGameObject)
            {
                auto pos = m_pGameObject->GetTransform().GetPosition();

				// feedback - deltaTime, use vector math
                glm::vec2 movement = m_Direction * (m_Speed * deltaTime);
                m_pGameObject->SetLocalPosition(pos.x + movement.x, pos.y + movement.y);

                // FLIPPING
                auto renderComp = m_pGameObject->GetComponent<dae::RenderComponent>();
                if (renderComp && m_Direction.x != 0)
                {
                    renderComp->SetFlip(m_Direction.x < 0);
                }
            }
        }


    private:
        GameObject* m_pGameObject;
        glm::vec2 m_Direction;
        float m_Speed;
    };
}

#endif