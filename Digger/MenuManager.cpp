#include "MenuManager.h"

#include "HighScoreManager.h"
#include "SceneManager.h"
#include "Scene.h"

#include <SDL3/SDL.h>

namespace dae
{
    MenuManager::MenuManager(GameObject* owner, HighScoreManager* pMgr, Scene* pScoreScene, Scene* pGameScene)
        : Component(owner)
        , m_pMgr(pMgr)
        , m_pScoreScene(pScoreScene)
        , m_pGameScene(pGameScene)
    {
    }

    void MenuManager::Update(float /*deltaTime*/)
    {
        // Transition to the game scene on any keyboard input
        const bool* pStates = SDL_GetKeyboardState(nullptr);

        for (int i = 0; i < SDL_SCANCODE_COUNT; ++i)
        {
            if (pStates[i])
            {
                if (m_pMgr && m_pMgr->HasSessionName())
                {
                    SceneManager::GetInstance().SetActiveScene(m_pGameScene);
                }
                else
                {
                    SceneManager::GetInstance().SetActiveScene(m_pScoreScene);
                }
                return;
            }
        }
    }
}
