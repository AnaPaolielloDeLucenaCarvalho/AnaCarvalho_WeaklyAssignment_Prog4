#ifndef MUTE_COMMAND_H
#define MUTE_COMMAND_H

#include "Command.h"

#include <cstdint>

namespace dae
{
    class MuteCommand final : public Command
    {
    public:
        MuteCommand() = default;

        void Execute(float deltaTime) override;

    private:
        uint64_t m_LastToggleTime{ 0 }; // replaces SDL Uint64 — same underlying type
    };
}

#endif // MUTE_COMMAND_H