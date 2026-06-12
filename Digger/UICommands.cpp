#include "UICommands.h"
#include "MenuManager.h"
#include "GameOverManager.h"
#include "SceneManager.h"
#include <SDL3/SDL.h>

namespace dae
{
    MenuNavigateCommand::MenuNavigateCommand(int direction, MenuManager* menuMgr, GameOverManager* goMgr, Scene* targetScene)
        : m_Direction(direction)
        , m_pMenuMgr(menuMgr)
        , m_pGameOverMgr(goMgr)
        , m_pTargetScene(targetScene)
    {
    }

    void MenuNavigateCommand::Execute(float /*deltaTime*/)
    {
        uint64_t currentTime = SDL_GetTicks();
        if (currentTime - m_LastPressTime < 200) return;
        m_LastPressTime = currentTime;

        if (dae::SceneManager::GetInstance().GetActiveScene() != m_pTargetScene) return;

        if (m_pMenuMgr)
        {
            if (m_Direction < 0) m_pMenuMgr->NavigateUp();
            else if (m_Direction > 0) m_pMenuMgr->NavigateDown();
        }
        else if (m_pGameOverMgr)
        {
            if (m_Direction < 0) m_pGameOverMgr->NavigateUp();
            else if (m_Direction > 0) m_pGameOverMgr->NavigateDown();
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
        uint64_t currentTime = SDL_GetTicks();
        if (currentTime - m_LastPressTime < 200) return;
        m_LastPressTime = currentTime;

        if (dae::SceneManager::GetInstance().GetActiveScene() != m_pTargetScene) return;

        if (m_pMenuMgr)
            m_pMenuMgr->Select();
        else if (m_pGameOverMgr)
            m_pGameOverMgr->Select();
    }
}
