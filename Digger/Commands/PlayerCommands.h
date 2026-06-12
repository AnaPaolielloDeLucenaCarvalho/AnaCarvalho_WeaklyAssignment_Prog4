#ifndef PLAYER_COMMANDS_H
#define PLAYER_COMMANDS_H

#include "Command.h"
#include <glm/vec2.hpp>

// DESIGN PATTERN - Command Pattern
// Using this pattern means the InputManager doesn't need to know what "Player" is, it just maps a physical key (makes Co-Op easy to implement, because I can just pass a pointer for Player 1 into one command, and a pointer for Player 2 into another) to a floating instruction

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
        glm::vec2 m_direction; // Direction - (0, -1) up, (0, 1) down, (-1, 0) left, (1, 0) right.
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
