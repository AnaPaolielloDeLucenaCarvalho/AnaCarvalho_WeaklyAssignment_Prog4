#include "UICommands.h"
#include "MenuManager.h"
#include "GameOverManager.h"
#include "SceneManager.h"
#include <SDL3/SDL.h>

namespace dae
{
    MenuNavigateCommand::MenuNavigateCommand(int direction, MenuManager* menuMgr, GameOverManager* goMgr, Scene* targetScene)
        : m_direction(direction)
        , m_pMenuMgr(menuMgr)
        , m_pGameOverMgr(goMgr)
        , m_pTargetScene(targetScene)
    {
    }

    void MenuNavigateCommand::Execute(float /*deltaTime*/)
    {
        // Cooldown Logic - We use SDL_GetTicks to prevent the user from accidentally jumping  past multiple menu options in a single keystroke.
        uint64_t currentTime = SDL_GetTicks();
        if (currentTime - m_lastPressTime < 200) return;
        m_lastPressTime = currentTime;

        // Scene Guard - Prevents inputs from bleeding into menus that are currently hidden
        if (dae::SceneManager::GetInstance().GetActiveScene() != m_pTargetScene) return;

        // We check which manager pointer is valid and route the navigation to the correct screen
        if (m_pMenuMgr)
        {
            if (m_direction < 0) m_pMenuMgr->NavigateUp();
            else if (m_direction > 0) m_pMenuMgr->NavigateDown();
        }
        else if (m_pGameOverMgr)
        {
            if (m_direction < 0) m_pGameOverMgr->NavigateUp();
            else if (m_direction > 0) m_pGameOverMgr->NavigateDown();
        }
    }

    MenuSelectCommand::MenuSelectCommand(MenuManager* menuMgr, GameOverManager* goMgr, Scene* targetScene)
        : m_pMenuMgr(menuMgr)
        , m_pGameOverMgr(goMgr)
        , m_pTargetScene(targetScene)
    {
    }

    void MenuSelectCommand::Execute(float /*deltaTime*/)
    {
        // Cooldown Logic - Prevent double-triggering a menu action by enforcing a 200ms cooldown.
        uint64_t currentTime = SDL_GetTicks();
        if (currentTime - m_lastPressTime < 200) return;
        m_lastPressTime = currentTime;

        // Scene Guard
        if (dae::SceneManager::GetInstance().GetActiveScene() != m_pTargetScene) return;

        // Route the select confirmation to whichever UI manager is actively loaded
        if (m_pMenuMgr)
        {
            m_pMenuMgr->Select();
        }
        else if (m_pGameOverMgr)
        {
            m_pGameOverMgr->Select();
        }
    }
}
