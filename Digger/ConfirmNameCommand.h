#ifndef CONFIRM_NAME_COMMAND_H
#define CONFIRM_NAME_COMMAND_H

#include "Command.h"

//  Fired when player presses Enter on name screen. Saves initials to HighScoreManager and transitions to game scene
namespace dae
{
    class NameEntryComponent;

    class ConfirmNameCommand final : public Command
    {
    public:
        explicit ConfirmNameCommand(NameEntryComponent* pEntry);

        void Execute(float deltaTime) override;

    private:
        NameEntryComponent* m_pEntry;
    };
}

#endif // CONFIRM_NAME_COMMAND_H
