#ifndef UICOMMANDS_H
#define UICOMMANDS_H

#include "Command.h"
#include <cstdint>

// DESIGN PATTERN - Command Pattern
// Used here to decouple the raw controller/keyboard input from the UI logic. Rather than the MenuManager polling SDL_GetKeyboardState directly, the input system creates these objects and hands them over, making UI safely navigable by any device.

namespace dae
{
    class MenuManager;
    class GameOverManager;

    class Scene;

// Command to navigate up/down in the menu or game over screen
    class MenuNavigateCommand final : public Command
    {
    public:
        MenuNavigateCommand(int direction, MenuManager* menuMgr, GameOverManager* goMgr, Scene* targetScene);
        void Execute(float /*deltaTime*/) override;

    private:
        int m_direction; // -1 for up, +1 for down
        MenuManager* m_pMenuMgr;
        GameOverManager* m_pGameOverMgr;
        Scene* m_pTargetScene;
        uint64_t m_lastPressTime{ 0 }; // Timestamp of the last button press for debouncing
    };

// Command to select the currently highlighted option in the menu or game over screen
    class MenuSelectCommand final : public Command
    {
    public:
        MenuSelectCommand(MenuManager* menuMgr, GameOverManager* goMgr, Scene* targetScene);
        void Execute(float deltaTime) override;

    private:
        MenuManager* m_pMenuMgr;
        GameOverManager* m_pGameOverMgr;
        Scene* m_pTargetScene;
        uint64_t m_lastPressTime{ 0 }; // Timestamp of the last button press for debouncing
    };
}

#endif // UICOMMANDS_H
