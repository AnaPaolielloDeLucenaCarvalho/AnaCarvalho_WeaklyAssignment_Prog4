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
        , m_options(options)
    {
    }

    void GameOverManager::Update(float /*deltaTime*/)
    {
        // One-time initialization block that dynamically alters the UI text based on the active game mode. Doing this here rather than the constructor guarantees we pull the most recent score string.
        if (!m_isSetup)
        {
            m_isSetup = true;
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

        // Iterate through all menu options and visually highlight the currently selected index in yellow
        for (size_t i = 0; i < m_options.size(); ++i)
        {
            if (static_cast<int>(i) == m_selectedIndex)
            {
                m_options[i]->SetColor(SDL_Color{ 255, 255, 0, 255 }); // Yellow
            }
            else
            {
                m_options[i]->SetColor(SDL_Color{ 255, 255, 255, 255 }); // White
            }
        }
    }

    void GameOverManager::NavigateUp()
    {
        // Wrap index backward to allow infinite looping navigation
        m_selectedIndex--;
        if (m_selectedIndex < 0) m_selectedIndex = static_cast<int>(m_options.size()) - 1;
    }

    void GameOverManager::NavigateDown()
    {
        // Wrap index forward to allow infinite looping navigation
        m_selectedIndex++;
        if (m_selectedIndex >= static_cast<int>(m_options.size())) m_selectedIndex = 0;
    }

    void GameOverManager::Select()
    {
        // Flag the level manager to perform a hard data reset so the player doesn't spawn dead
        LevelManager::GetInstance().SetNeedsGameReset(true);

        if (m_selectedIndex == 0) // "TRY AGAIN"
        {
            SceneManager::GetInstance().SetActiveScene(m_pGameScene);
        }
        else if (m_selectedIndex == 1) // "MAIN MENU"
        {
            // Crucial - Wipe the active player initials from memory so the next game prompts for a new name!
            if (m_pHighScoreMgr) m_pHighScoreMgr->ClearSessionName();
            SceneManager::GetInstance().SetActiveScene(m_pMenuScene);
        }

        // Reset the setup flag so it correctly updates the text strings next time this scene is loaded
        m_isSetup = false;
    }
}