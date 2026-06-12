#ifndef SHOOTCOMMAND_H
#define SHOOTCOMMAND_H

#include "Command.h"

namespace dae
{
    class DiggerComponent; // forward declaration — full type only needed in .cpp

    class ShootCommand final : public Command
    {
    public:
        explicit ShootCommand(DiggerComponent* pDigger);

        void Execute(float deltaTime) override;

    private:
        DiggerComponent* m_pDigger;
    };
}

#endif // SHOOTCOMMAND_H