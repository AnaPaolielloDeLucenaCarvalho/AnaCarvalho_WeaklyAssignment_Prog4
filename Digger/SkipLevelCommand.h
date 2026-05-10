#ifndef SKIP_LEVEL_COMMAND_H
#define SKIP_LEVEL_COMMAND_H

#include "Command.h"
#include "DiggerComponent.h"
#include "Observer.h"

namespace dae
{
    class SkipLevelCommand final : public Command
    {
    public:
        SkipLevelCommand(DiggerComponent* pDigger) : m_pDigger(pDigger) {}

        void Execute(float /*deltaTime*/) override
        {
            if (m_pDigger)
            {
                m_pDigger->GetSubject().Notify(make_sdbm_hash("LoadNextLevel"), 0);
            }
        }
    private:
        DiggerComponent* m_pDigger;
    };
}
#endif