#ifndef NAME_ENTRY_COMMANDS_H
#define NAME_ENTRY_COMMANDS_H

#include "Command.h"

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

    class ConfirmNameCommand final : public Command
    {
    public:
        explicit ConfirmNameCommand(NameEntryComponent* pEntry, Scene* targetScene);
        void Execute(float deltaTime) override;

    private:
        NameEntryComponent* m_pEntry;
        Scene* m_pTargetScene;
    };

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

#endif // NAME_ENTRY_COMMANDS_H
