#include "GameOverManager.h"
#include "TextComponent.h"
#include "SceneManager.h"
#include "LevelManager.h"
#include "HighScoreManager.h"
#include <SDL3/SDL.h>
#include <sstream>
#include <iomanip>

namespace dae
{
    GameOverManager::GameOverManager(GameObject* pOwner, Scene* menuScene, Scene* gameScene, HighScoreManager* highScoreMgr, TextComponent* titleText, TextComponent* scoreText, const std::vector<TextComponent*>& options)
        : Component(pOwner)
        , m_pMenuScene(menuScene)
        , m_pGameScene(gameScene)
        , m_pHighScoreMgr(highScoreMgr)
        , m_pTitleText(titleText)
        , m_pScoreText(scoreText)
        , m_Options(options)
    {
    }

    void GameOverManager::Update(float deltaTime)
    {
        if (m_InputCooldown > 0.0f)
        {
            m_InputCooldown -= deltaTime;
        }

        if (!m_IsSetup)
        {
            m_IsSetup = true;
            if (LevelManager::GetInstance().GetGameMode() == GameMode::Versus)
            {
                m_pTitleText->SetText("VERSUS OVER");
                m_pScoreText->SetText(LevelManager::GetInstance().GetWinnerText());
            }
            else
            {
                m_pTitleText->SetText("GAME OVER");
                std::ostringstream ss;
                ss << "FINAL SCORE: " << m_pHighScoreMgr->GetCurrentScore();
                m_pScoreText->SetText(ss.str());
            }
        }

        const bool* state = SDL_GetKeyboardState(nullptr);
        
        if (m_InputCooldown <= 0.0f)
        {
            if (state[SDL_SCANCODE_UP])
            {
                m_SelectedIndex--;
                if (m_SelectedIndex < 0) m_SelectedIndex = static_cast<int>(m_Options.size()) - 1;
                m_InputCooldown = 0.2f;
            }
            else if (state[SDL_SCANCODE_DOWN])
            {
                m_SelectedIndex++;
                if (m_SelectedIndex >= static_cast<int>(m_Options.size())) m_SelectedIndex = 0;
                m_InputCooldown = 0.2f;
            }
            else if (state[SDL_SCANCODE_RETURN])
            {
                m_InputCooldown = 0.5f;
                LevelManager::GetInstance().SetNeedsGameReset(true);
                
                if (m_SelectedIndex == 0)
                {
                    SceneManager::GetInstance().SetActiveScene(m_pGameScene);
                }
                else if (m_SelectedIndex == 1)
                {
                    if (m_pHighScoreMgr) m_pHighScoreMgr->ClearSessionName(); // Forget the name!
                    SceneManager::GetInstance().SetActiveScene(m_pMenuScene);
                }
                
                m_IsSetup = false;
                return;
            }
        }

        for (size_t i = 0; i < m_Options.size(); ++i)
        {
            if (static_cast<int>(i) == m_SelectedIndex)
            {
                m_Options[i]->SetColor(SDL_Color{255, 255, 0, 255}); // Yellow
            }
            else
            {
                m_Options[i]->SetColor(SDL_Color{255, 255, 255, 255}); // White
            }
        }
    }
}
