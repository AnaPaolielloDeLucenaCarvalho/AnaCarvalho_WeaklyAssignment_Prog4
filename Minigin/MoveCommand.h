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
        MoveCommand(GameObject* gameObject, const glm::vec2& direction, float speed, float gridSize = 40.0f, float offsetY = 52.0f)
            : m_pGameObject(gameObject)
            , m_Direction(direction)
            , m_Speed(speed)
            , m_GridSize{ gridSize }
            , m_OffsetY{ offsetY }
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
                float newX = pos.x + movement.x;
                float newY = pos.y + movement.y;

                float spriteOffsetX = 4.0f;
                float spriteOffsetY = 8.0f;

				// SNAP TO GRID
                if (m_Direction.x != 0.0f)
                {
                    // Left/Right = Y
                    newY = std::round((pos.y - m_OffsetY - spriteOffsetY) / m_GridSize) * m_GridSize + m_OffsetY + spriteOffsetY;
                }
                else if (m_Direction.y != 0.0f)
                {
                    // Up/Down = X
                    newX = std::round((pos.x - spriteOffsetX) / m_GridSize) * m_GridSize + spriteOffsetX;
                }

                m_pGameObject->SetLocalPosition(newX, newY);

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
        float m_GridSize;
        float m_OffsetY;
    };
}

#endif