#include "SkipLevelCommand.h"

#include "DiggerComponent.h"
#include "Observer.h"
#include <SDL3/SDL.h>

namespace dae
{
    SkipLevelCommand::SkipLevelCommand(DiggerComponent* pDigger)
        : m_pDigger(pDigger)
    {
    }

    void SkipLevelCommand::Execute(float /*deltaTime*/)
    {
        uint64_t currentTime = SDL_GetTicks();
        if (currentTime - m_LastPressTime < 300) return;
        m_LastPressTime = currentTime;

        if (m_pDigger)
        {
            m_pDigger->GetSubject().Notify(make_sdbm_hash("LoadNextLevel"), 0);
        }
    }
}
