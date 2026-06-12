#ifndef SKIP_LEVEL_COMMAND_H
#define SKIP_LEVEL_COMMAND_H

#include "Command.h"
#include <cstdint>

namespace dae
{
    class DiggerComponent; // forward declaration — full type only needed in .cpp

    class SkipLevelCommand final : public Command
    {
    public:
        explicit SkipLevelCommand(DiggerComponent* pDigger);

        void Execute(float deltaTime) override;

    private:
        DiggerComponent* m_pDigger;
        uint64_t m_LastPressTime{ 0 };
    };
}

#endif // SKIP_LEVEL_COMMAND_H