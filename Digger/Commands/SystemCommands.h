#ifndef SYSTEM_COMMANDS_H
#define SYSTEM_COMMANDS_H

#include "Command.h"
#include <cstdint>

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
}
#endif
