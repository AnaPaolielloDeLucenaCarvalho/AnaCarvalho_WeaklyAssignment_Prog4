#include "MenuManager.h"

#include "HighScoreManager.h"
#include "SceneManager.h"
#include "Scene.h"
#include "LevelManager.h"

#include <SDL3/SDL.h>

namespace dae
{
    MenuManager::MenuManager(GameObject* owner, HighScoreManager* pMgr, Scene* pScoreScene, Scene* pGameScene, const std::vector<TextComponent*>& options)
        : Component(owner)
        , m_pMgr(pMgr)
        , m_pScoreScene(pScoreScene)
        , m_pGameScene(pGameScene)
        , m_Options(options)
    {
    }

    void MenuManager::Update(float deltaTime)
    {
        if (m_InputCooldown > 0.0f)
        {
            m_InputCooldown -= deltaTime;
        }

        const bool* pStates = SDL_GetKeyboardState(nullptr);

        if (m_InputCooldown <= 0.0f)
        {
            if (pStates[SDL_SCANCODE_DOWN])
            {
                m_SelectedIndex++;
                if (m_SelectedIndex > 2) m_SelectedIndex = 0;
                m_InputCooldown = 0.2f;
            }
            else if (pStates[SDL_SCANCODE_UP])
            {
                m_SelectedIndex--;
                if (m_SelectedIndex < 0) m_SelectedIndex = 2;
                m_InputCooldown = 0.2f;
            }
        }

        for (int i = 0; i < static_cast<int>(m_Options.size()); ++i)
        {
            if (i == m_SelectedIndex)
            {
                m_Options[i]->SetColor(SDL_Color{ 255, 255, 0, 255 }); // Yellow
            }
            else
            {
                m_Options[i]->SetColor(SDL_Color{ 255, 255, 255, 255 }); // White
            }
        }

        if (pStates[SDL_SCANCODE_RETURN] && m_InputCooldown <= 0.0f)
        {
            GameMode mode = GameMode::SinglePlayer;
            if (m_SelectedIndex == 1) mode = GameMode::CoOp;
            else if (m_SelectedIndex == 2) mode = GameMode::Versus;

            LevelManager::GetInstance().SetGameMode(mode);

            if (m_pMgr && m_pMgr->HasSessionName())
            {
                SceneManager::GetInstance().SetActiveScene(m_pGameScene);
            }
            else
            {
                SceneManager::GetInstance().SetActiveScene(m_pScoreScene);
            }
        }
    }
}
