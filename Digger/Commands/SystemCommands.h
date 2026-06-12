#ifndef SYSTEM_COMMANDS_H
#define SYSTEM_COMMANDS_H

#include "Command.h"
#include <cstdint>

// DESIGN PATTERN - Command Pattern
// By encapsulating these as objects, we avoid hardcoding raw SDL_SCANCODE checks deep inside the engine's core update loop. It keeps the architecture modular so hotkeys can be safely assigned to either the keyboard or a gamepad.

namespace dae
{
    class Scene;
    class HighScoreManager;

    class ReturnToMenuCommand final : public Command
    {
    public:
        explicit ReturnToMenuCommand(Scene* menuScene, HighScoreManager* highScoreManager);
        void Execute(float deltaTime) override;

    private:
        Scene* m_pMenuScene;
        HighScoreManager* m_pHighScoreManager;
    };

    class ToggleInstructionsCommand final : public Command
    {
    public:
        explicit ToggleInstructionsCommand(Scene* instructionsScene);
        void Execute(float deltaTime) override;

    private:
        Scene* m_pInstructionsScene;
        Scene* m_pPreviousScene{ nullptr };
        uint64_t m_LastPressTime{ 0 };
    };

    class MuteCommand final : public Command
    {
    public:
        MuteCommand() = default;
        void Execute(float deltaTime) override;
    private:
        uint64_t m_LastToggleTime{ 0 };
    };

    class DiggerComponent;

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
#endif
