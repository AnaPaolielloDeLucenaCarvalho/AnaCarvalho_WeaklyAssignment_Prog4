#include "MuteCommand.h"

#include "ServiceLocator.h"

#include <SDL3/SDL_timer.h>

namespace dae
{
    void MuteCommand::Execute(float /*deltaTime*/)
    {
        const uint64_t currentTime = SDL_GetTicks();

        // 300 ms debounce to prevent rapid toggling on held key
        if (currentTime - m_LastToggleTime > 300)
        {
            ServiceLocator::GetSoundSystem().ToggleMute();
            m_LastToggleTime = currentTime;
        }
    }
}
