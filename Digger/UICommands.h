#ifndef UICOMMANDS_H
#define UICOMMANDS_H

#include "Command.h"
#include <cstdint>

namespace dae
{
    class MenuManager;
    class GameOverManager;

    class Scene;

    class MenuNavigateCommand final : public Command
    {
    public:
        MenuNavigateCommand(int direction, MenuManager* menuMgr, GameOverManager* goMgr, Scene* targetScene);
        void Execute(float /*deltaTime*/) override;

    private:
        int m_Direction;
        MenuManager* m_pMenuMgr;
        GameOverManager* m_pGameOverMgr;
        Scene* m_pTargetScene;
        uint64_t m_LastPressTime{ 0 };
    };

    class MenuSelectCommand final : public Command
    {
    public:
        MenuSelectCommand(MenuManager* menuMgr, GameOverManager* goMgr, Scene* targetScene);
        void Execute(float deltaTime) override;

    private:
        MenuManager* m_pMenuMgr;
        GameOverManager* m_pGameOverMgr;
        Scene* m_pTargetScene;
        uint64_t m_LastPressTime{ 0 };
    };
}

#endif // UICOMMANDS_H
