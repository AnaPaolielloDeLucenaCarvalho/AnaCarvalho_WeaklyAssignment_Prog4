#ifndef ADVANCE_INDEX_COMMAND_H
#define ADVANCE_INDEX_COMMAND_H

#include "Command.h"

//  Moves the cursor to the next initial on the name-entry screen (Right/D).
namespace dae
{
    class NameEntryComponent;

    class Scene;

    class AdvanceIndexCommand final : public Command
    {
    public:
        explicit AdvanceIndexCommand(NameEntryComponent* pEntry, int direction, Scene* targetScene);

        void Execute(float deltaTime) override;

    private:
        NameEntryComponent* m_pEntry;
        int m_Direction;
        Scene* m_pTargetScene;
    };

}

#endif // ADVANCE_INDEX_COMMAND_H
