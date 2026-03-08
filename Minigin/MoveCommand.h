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
			// Later ill add deltatime to this
            auto pos = m_pGameObject->GetTransform().GetPosition();
            m_pGameObject->SetLocalPosition(pos.x + (m_Direction.x * m_Speed), pos.y + (m_Direction.y * m_Speed));
        }

    private:
        GameObject* m_pGameObject;
        glm::vec2 m_Direction;
        float m_Speed;
    };
}

#endif