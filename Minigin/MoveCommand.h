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
        }

        void Execute() override
        {
            if (m_pGameObject)
            {
                auto pos = m_pGameObject->GetTransform().GetPosition();
                const float gridSize = 16.0f; // digger used 16 grid

                if (m_Direction.x != 0) // horizontal
                {
                    pos.y = std::round(pos.y / gridSize) * gridSize;
                    pos.x += m_Direction.x * m_Speed;
                }
                else if (m_Direction.y != 0) // vertical
                {
                    pos.x = std::round(pos.x / gridSize) * gridSize;
                    pos.y += m_Direction.y * m_Speed;
                }

                m_pGameObject->SetLocalPosition(pos.x, pos.y);

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