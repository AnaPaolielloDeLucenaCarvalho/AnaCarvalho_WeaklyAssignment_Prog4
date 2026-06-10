#ifndef ADVANCE_INDEX_COMMAND_H
#define ADVANCE_INDEX_COMMAND_H

#include "Command.h"

//  Moves the cursor to the next initial on the name-entry screen (Right/D).
namespace dae
{
    class NameEntryComponent;

    class AdvanceIndexCommand final : public Command
    {
    public:
        explicit AdvanceIndexCommand(NameEntryComponent* pEntry);

        void Execute(float deltaTime) override;

    private:
        NameEntryComponent* m_pEntry;
    };

}

#endif // ADVANCE_INDEX_COMMAND_H
