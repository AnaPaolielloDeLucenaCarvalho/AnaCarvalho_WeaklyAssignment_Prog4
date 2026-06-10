#ifndef MOVECOMMAND_H
#define MOVECOMMAND_H

#include "Command.h"

#include <glm/vec2.hpp>

namespace dae
{
    class DiggerComponent; // forward declaration — full type only needed in .cpp

    class MoveCommand final : public Command
    {
    public:
        MoveCommand(DiggerComponent* pDigger, const glm::vec2& direction);

        void Execute(float deltaTime) override;

    private:
        DiggerComponent* m_pDigger;
        glm::vec2 m_Direction;
    };
}

#endif // MOVECOMMAND_H