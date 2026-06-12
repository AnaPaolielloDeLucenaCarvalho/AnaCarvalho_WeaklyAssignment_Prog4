#ifndef CYCLE_LETTER_COMMAND_H
#define CYCLE_LETTER_COMMAND_H

#include "Command.h"

//  Cycles selected initial on name screen - Right/D = A→B ,Left/A = B→A
namespace dae
{
    class NameEntryComponent;
    class Scene;

    class CycleLetterCommand final : public Command
    {
    public:
        CycleLetterCommand(NameEntryComponent* pEntry, int direction, Scene* targetScene);

        void Execute(float deltaTime) override;

    private:
        NameEntryComponent* m_pEntry;
        int m_Direction;
        Scene* m_pTargetScene;
    };

}

#endif // CYCLE_LETTER_COMMAND_H
