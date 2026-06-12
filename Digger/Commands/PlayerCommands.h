#ifndef PLAYER_COMMANDS_H
#define PLAYER_COMMANDS_H

#include "Command.h"
#include <glm/vec2.hpp>

namespace dae
{
    class DiggerComponent;

    class MoveCommand final : public Command
    {
    public:
        MoveCommand(DiggerComponent* pDigger, const glm::vec2& direction);
        void Execute(float deltaTime) override;

    private:
        DiggerComponent* m_pDigger;
        glm::vec2 m_Direction;
    };

    class ShootCommand final : public Command
    {
    public:
        explicit ShootCommand(DiggerComponent* pDigger);
        void Execute(float deltaTime) override;

    private:
        DiggerComponent* m_pDigger;
    };
}

#endif // PLAYER_COMMANDS_H
