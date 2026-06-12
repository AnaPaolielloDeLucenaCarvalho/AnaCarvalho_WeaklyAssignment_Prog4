#ifndef NAME_ENTRY_COMMANDS_H
#define NAME_ENTRY_COMMANDS_H

#include "Command.h"

// DESIGN PATTERN - Command Pattern
// Used the Command pattern here to decouple the inputs from the Name Entry logic, the Input Manager calls Execute() on the Command objects, which then calls the appropriate functions on the NameEntryComponent.
// Why - Prevents the code from becoming a massive switch statement in Update and makes controller rebinding trivial

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
        int m_direction;
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
        int m_direction;
        Scene* m_pTargetScene;
    };
}

#endif // NAME_ENTRY_COMMANDS_H
