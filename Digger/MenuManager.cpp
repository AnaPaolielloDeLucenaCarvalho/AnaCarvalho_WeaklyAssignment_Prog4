#include "MenuManager.h"

#include "HighScoreManager.h"
#include "SceneManager.h"
#include "Scene.h"
#include "LevelManager.h"

#include <SDL3/SDL.h>
#include <sstream>
#include <iomanip>

namespace dae
{
    MenuManager::MenuManager(GameObject* owner, HighScoreManager* pMgr, Scene* pScoreScene, Scene* pGameScene, 
                             const std::vector<TextComponent*>& options,
                             const std::vector<TextComponent*>& scoreTexts,
                             const std::vector<TextComponent*>& nameTexts)
        : Component(owner)
        , m_pMgr(pMgr)
        , m_pScoreScene(pScoreScene)
        , m_pGameScene(pGameScene)
        , m_Options(options)
        , m_ScoreTexts(scoreTexts)
        , m_NameTexts(nameTexts)
    {
    }

    void MenuManager::Update(float deltaTime)
    {
        // Refresh the Leaderboard periodically while the menu is active
        m_RefreshTimer -= deltaTime;
        if (m_RefreshTimer <= 0.0f && m_pMgr)
        {
            m_RefreshTimer = 1.0f; // Check every 1 second
            auto topScores = m_pMgr->GetTopScores(5);
            for (size_t i = 0; i < topScores.size() && i < m_ScoreTexts.size(); ++i)
            {
                std::ostringstream scoreOss;
                scoreOss << std::setfill('0') << std::setw(5) << topScores[i].score;
                if (m_ScoreTexts[i]) m_ScoreTexts[i]->SetText(scoreOss.str());
                if (m_NameTexts[i]) m_NameTexts[i]->SetText(topScores[i].initials);
            }
        }

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
            m_InputCooldown = 0.5f;

            // Force a hard reset for EVERY new game started from the menu!
            LevelManager::GetInstance().SetNeedsGameReset(true);

            if (m_SelectedIndex == 2)
            {
                LevelManager::GetInstance().SetGameMode(GameMode::Versus);
                SceneManager::GetInstance().SetActiveScene(m_pGameScene);
            }
            else
            {
                LevelManager::GetInstance().SetGameMode(m_SelectedIndex == 0 ? GameMode::SinglePlayer : GameMode::CoOp);

                if (m_pMgr && !m_pMgr->HasSessionName())
                {
                    SceneManager::GetInstance().SetActiveScene(m_pScoreScene);
                }
                else
                {
                    SceneManager::GetInstance().SetActiveScene(m_pGameScene);
                }
            }
        }
    }
}
