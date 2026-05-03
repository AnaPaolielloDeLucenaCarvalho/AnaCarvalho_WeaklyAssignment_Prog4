#ifndef MUTE_COMMAND_H
#define MUTE_COMMAND_H

#include "Command.h"
#include "ServiceLocator.h"
#include <SDL3/SDL_timer.h>

namespace dae
{
    class MuteCommand final : public Command
    {
        Uint64 m_LastToggleTime{ 0 };
    public:
        MuteCommand() = default;

        void Execute(float /*deltaTime*/) override
        {
            Uint64 currentTime = SDL_GetTicks();

            if (currentTime - m_LastToggleTime > 300)
            {
                ServiceLocator::get_sound_system().ToggleMute();
                m_LastToggleTime = currentTime;
            }
        }
    };
}
#endif