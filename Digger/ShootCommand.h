#ifndef SHOOTCOMMAND_H
#define SHOOTCOMMAND_H

#include "Command.h"
#include "DiggerComponent.h"

namespace dae
{
    class ShootCommand final : public Command
    {
    public:
        ShootCommand(DiggerComponent* pDigger) : m_pDigger(pDigger) {}

        void Execute(float /*deltaTime*/) override
        {
            if (m_pDigger)
            {
                m_pDigger->Shoot();
            }
        }
    private:
        DiggerComponent* m_pDigger;
    };
}
#endif